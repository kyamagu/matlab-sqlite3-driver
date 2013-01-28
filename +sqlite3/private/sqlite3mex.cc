// Sqlite3 matlab driver library.
//
// Kota Yamaguchi 2012 <kyamagu@cs.stonybrook.edu>

#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <set>
#include <sstream>
#include "sqlite3mex.h"

using boost::xpressive::sregex;

namespace sqlite3mex {

Statement::Statement() : statement_(NULL) {}

Statement::~Statement() {
  finalize();
}

bool Statement::prepare(const string& statement, sqlite3* database) {
  code_ = sqlite3_prepare_v2(database,
                             statement.c_str(),
                             statement.length() + 1,
                             &statement_,
                             NULL);
  return ok();
}

bool Statement::finalize() {
  if (statement_)
    code_ = sqlite3_finalize(statement_);
  statement_ = NULL;
  return ok();
}

bool Statement::step() {
  if (!statement_)
    return false;
  code_ = sqlite3_step(statement_);
  return row();
}

bool Statement::reset() {
  if (!statement_)
    return false;
  code_ = sqlite3_reset(statement_);
  return ok();
}

bool Statement::bind(const vector<const mxArray*>& params) {
  int num_binds = sqlite3_bind_parameter_count(statement_);
  if (params.size() != num_binds)
    ERROR("Wrong number of parameters: %d for %d.", params.size(), num_binds);
  code_ = sqlite3_clear_bindings(statement_);
  if (!ok())
    false;
  for (int i = 0; i < params.size(); ++i) {
    if (mxGetNumberOfElements(params[i]) == 1 && mxIsNumeric(params[i])) {
      if (mxIsDouble(params[i]) || mxIsSingle(params[i]))
        code_ = sqlite3_bind_double(statement_, i + 1, mxGetScalar(params[i]));
      else
        code_ = sqlite3_bind_int64(statement_, i + 1, mxGetScalar(params[i]));
    }
    else if (mxIsChar(params[i])) {
      char* str = mxArrayToString(params[i]);
      code_ = sqlite3_bind_text(statement_,
                                i + 1,
                                str,
                                -1,
                                SQLITE_TRANSIENT);
      mxFree(str);
    }
    else if (mxIsUint8(params[i])) {
      code_ = sqlite3_bind_blob(statement_,
                                i + 1,
                                mxGetData(params[i]),
                                mxGetNumberOfElements(params[i]),
                                SQLITE_STATIC);
    }
    else if (mxIsEmpty(params[i])) {
      code_ = sqlite3_bind_null(statement_, i + 1);
    }
    else
      return false;
    if (!ok())
      return false;
  }
  return true;
}

bool Statement::ok() const {
  return code_ == SQLITE_OK;
}

bool Statement::done() const {
  return code_ == SQLITE_DONE;
}

bool Statement::row() const {
  return code_ == SQLITE_ROW;
}

int Statement::code() const {
  return code_;
}

sqlite3_stmt* Statement::get() {
  return statement_;
}

int Statement::column_count() const {
  return sqlite3_column_count(statement_);
}

int Statement::data_count() const {
  return sqlite3_data_count(statement_);
}

const char* Statement::column_name(int i) const {
  return sqlite3_column_name(statement_, i);
}

int Statement::column_type(int i) const {
  return sqlite3_column_type(statement_, i);
}

const Value& Statement::column_value(int i) {
  // It is possible to directly create an mxArray* here. However, due to the
  // memory allocation pattern in Matlab, it is faster to keep the result into
  // a temporary storage and convert the values to mxArray* after we find
  // the number of rows.
  switch (column_type(i)) {
    case SQLITE_INTEGER: {
      value_.first = SQLITE_INTEGER;
      value_.second = static_cast<IntegerValue>(
          sqlite3_column_int64(statement_, i));
      break;
    }
    case SQLITE_FLOAT: {
      value_.first = SQLITE_FLOAT;
      value_.second = static_cast<FloatValue>(
          sqlite3_column_double(statement_, i));
      break;
    }
    case SQLITE_TEXT: {
      value_.first = SQLITE_TEXT;
      value_.second = static_cast<TextValue>(reinterpret_cast<const char*>(
          sqlite3_column_text(statement_, i)));
      break;
    }
    case SQLITE_BLOB: {
      const uint8_t* blob = reinterpret_cast<const uint8_t*>(
          sqlite3_column_blob(statement_, i));
      int bytes = sqlite3_column_bytes(statement_, i);
      value_.first = SQLITE_BLOB;
      value_.second = BlobValue(blob, blob + bytes);
      break;
    }
    case SQLITE_NULL: {
      value_.first = SQLITE_NULL;
      value_.second = NullValue();
      break;
    }
  }
  return value_;
}

StatementCache::StatementCache(size_t cache_size) : cache_size_(cache_size) {}

StatementCache::~StatementCache() {}

Statement* StatementCache::get(const string& statement, sqlite3* database) {
  CacheMap::iterator entry = table_.find(statement);
  // Cache miss.
  if (entry == table_.end()) {
    if (!table_[statement].prepare(statement, database)) {
      table_.erase(statement);
      return NULL;
    }
    fifo_.push_front(statement);
    entry = table_.find(statement);
    if (fifo_.size() > cache_size_) {
      table_.erase(fifo_.back());
      fifo_.pop_back();
    }
  }
  return &entry->second;
}

Database::Database() : database_(NULL) {}

Database::~Database() {
  close();
}

bool Database::open(const string& filename) {
  return sqlite3_open(filename.c_str(), &database_) == SQLITE_OK;
}

void Database::close() {
  if (database_) {
    sqlite3_close(database_);
    database_ = NULL;
  }
}

int Database::error_code() {
  return sqlite3_errcode(database_);
}

const char* Database::error_message() {
  return sqlite3_errmsg(database_);
}

bool Database::execute(const string& statement_string, 
                       const vector<const mxArray*>& params,
                       mxArray** result) {
  if (!result)
    return false;
  Statement* statement = statement_cache_.get(statement_string, database_);
  if (!statement || !statement->reset() || !statement->bind(params))
    return false;
  vector<Column> columns;
  bool first_row = true;
  while (statement->step()) {
    if (first_row) {
      create_columns(*statement, &columns);
      first_row = false;
    }
    for (int i = 0; i < statement->column_count(); ++i)
      columns[i].values.push_back(statement->column_value(i));
  }
  // TODO: check if the columns are valid.
  return statement->done() && convert_columns_to_array(&columns, result);
}

bool Database::busy_timeout(int milliseconds) {
  return sqlite3_busy_timeout(database_, milliseconds) == SQLITE_OK;
}

void Database::create_columns(const Statement& statement,
                              vector<Column>* columns) {
  columns->resize(statement.column_count());
  set<string> unique_names;
  for (int i = 0; i < statement.column_count(); ++i) {
    // Clean the column name.
    static const sregex leading_non_alphabets = sregex::compile("^[^a-zA-Z]*");
    static const sregex non_alphanumerics = sregex::compile("[^a-zA-Z0-9]+");
    string name(statement.column_name(i));
    name = boost::xpressive::regex_replace(name, leading_non_alphabets, "");
    name = boost::xpressive::regex_replace(name, non_alphanumerics, " ");
    boost::algorithm::trim(name);
    boost::algorithm::replace_all(name, " ", "_");
    boost::algorithm::to_lower(name);
    if (name.empty())
      name = "field";
    // Find a unique name for the duplicated column.
    int index = 0;
    string original_name(name);
    while (unique_names.find(name) != unique_names.end()) {
      stringstream name_builder(stringstream::in | stringstream::out);
      name_builder << original_name << "_" << ++index;
      name = name_builder.str();
    }
    unique_names.insert(name);
    (*columns)[i].name = name;
  }
}

bool Database::convert_columns_to_array(vector<Column>* columns,
                                        mxArray** array) {
  if (!array)
    return false;
  if (columns->empty()) {
    *array = mxCreateDoubleMatrix(0, 0, mxREAL);
  }
  else {
    vector<const char*> fieldnames;
    fieldnames.reserve(columns->size());
    for (vector<Column>::iterator it = columns->begin();
         it != columns->end(); ++it)
      fieldnames.push_back(it->name.c_str());
    *array = mxCreateStructMatrix(1, (*columns)[0].values.size(),
                                  columns->size(), &fieldnames[0]);
    for (size_t i = 0; i < columns->size(); ++i){
      deque<Value>* values = &(*columns)[i].values;
      mwSize j = 0;
      while (!values->empty()) {
        mxArray* element = convert_value_to_array(values->front());
        mxSetFieldByNumber(*array, j++, i, element);
        values->pop_front();
      }
    }
  }
  return true;
}

mxArray* Database::convert_value_to_array(const Value& value) {
  mxArray* array = NULL;
  switch (value.first) {
    case SQLITE_INTEGER: {
      // Integer types in matlab are very restricted. Convert them to double
      // by default.
      //array = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
      //*reinterpret_cast<IntegerValue*>(mxGetData(array)) =
      //    static_cast<IntegerValue>(boost::get<IntegerValue>(value));
      array = mxCreateDoubleScalar(boost::get<IntegerValue>(value.second));
      break;
    }
    case SQLITE_FLOAT: {
      array = mxCreateDoubleScalar(boost::get<FloatValue>(value.second));
      break;
    }
    case SQLITE_TEXT: {
      array = mxCreateString(boost::get<TextValue>(value.second).c_str());
      break;
    }
    case SQLITE_BLOB: {
      const BlobValue& blob = boost::get<BlobValue>(value.second);
      array = mxCreateNumericMatrix(1, blob.size(), mxUINT8_CLASS, mxREAL);
      copy(blob.begin(), blob.end(),
           reinterpret_cast<uint8_t*>(mxGetData(array)));
      break;
    }
    case SQLITE_NULL: {
      array = mxCreateDoubleMatrix(0, 0, mxREAL);
      break;
    }
  }
  if (!array)
    ERROR("Failed to create mxArray.");
  return array;
}

DatabaseManager::DatabaseManager() : last_id_(0) {}

DatabaseManager::~DatabaseManager() {}

int DatabaseManager::open(const string& filename) {
  if (!connections_[++last_id_].open(filename))
    ERROR("Unable to open: %s.", filename.c_str());
  return last_id_;
}

void DatabaseManager::close(int id) {
  connections_.erase(id);
}

int DatabaseManager::default_id() const {
  return (connections_.empty()) ? 0 : connections_.rbegin()->first;
}

int DatabaseManager::last_id() const {
  return last_id_;
}

Database* DatabaseManager::get(int id) {
  map<int, Database>::iterator connection = connections_.find(id);
  if (connection == connections_.end())
    ERROR("Invalid id: %d", id);
  return &connection->second;
}

DatabaseManager Operation::manager_ = DatabaseManager();

Operation* Operation::parse(int nrhs, const mxArray *prhs[]) {
  if (nrhs < 1)
    ERROR("Operation missing.");
  if (!mxIsChar(prhs[0]))
    ERROR("Invalid operation.");

  auto_ptr<Operation> operation;
  string operation_name(mxGetChars(prhs[0]),
                        mxGetChars(prhs[0]) + mxGetNumberOfElements(prhs[0]));
  vector<const mxArray*> args(prhs + 1, prhs + nrhs);

  if (operation_name == "open")
    operation.reset(new OpenOperation());
  else if (operation_name == "close")
    operation.reset(new CloseOperation());
  else if (operation_name == "execute")
    operation.reset(new ExecuteOperation());
  else if (operation_name == "timeout")
    operation.reset(new TimeoutOperation());
  else
    ERROR("Invalid operation: %s.", operation_name.c_str());

  operation->parse_internal(args);
  return operation.release();
}

void OpenOperation::parse_internal(const vector<const mxArray*>& args) {
  if (args.empty())
    ERROR("Missing filename.");
  if (args.size() > 1)
    ERROR("Too many input: %d for 1.", args.size());
  if (!mxIsChar(args[0]))
    ERROR("Invalid filename.");
  filename_.assign(mxGetChars(args[0]),
                   mxGetChars(args[0]) + mxGetNumberOfElements(args[0]));
}

void OpenOperation::run(int nlhs, mxArray* plhs[]) {
  if (nlhs > 1)
    ERROR("Too many output: %d for 1.", nlhs);
  plhs[0] = mxCreateDoubleScalar(manager()->open(filename_));
}

void CloseOperation::parse_internal(const vector<const mxArray*>& args) {
  if (args.size() > 1)
    ERROR("Too many input.");
  if (args.size() == 1 && mxIsDouble(args[0]))
    id_ = mxGetScalar(args[0]);
  else
    id_ = manager()->default_id();
}

void CloseOperation::run(int nlhs, mxArray* plhs[]) {
  if (nlhs)
    ERROR("Too many output: %d for 0.", nlhs);
  manager()->close(id_);
}

void ExecuteOperation::parse_internal(const vector<const mxArray*>& args) {
  vector<const mxArray*>::const_iterator it = args.begin();
  if (it == args.end())
    ERROR("Missing input.");
  if (mxIsDouble(*it)) {
    id_ = mxGetScalar(*it);
    ++it;
  }
  else
    id_ = manager()->default_id();
  if (it == args.end() || !mxIsChar(*it))
    ERROR("Missing sql statement.");
  sql_.assign(mxGetChars(*it),
              mxGetChars(*it) + mxGetNumberOfElements(*it));
  params_.assign(++it, args.end());
}

void ExecuteOperation::run(int nlhs, mxArray* plhs[]) {
  if (nlhs > 1)
    ERROR("Too many output: %d for 1.", nlhs);
  sqlite3mex::Database* connection = manager()->get(id_);
  if (!connection->execute(sql_, params_, &plhs[0]))
    ERROR("%s: %s", connection->error_message(), sql_.c_str());
}

void TimeoutOperation::parse_internal(const vector<const mxArray*>& args) {
  if (args.size() == 1 && mxIsDouble(args[0])) {
    id_ = manager()->default_id();
    timeout_ = mxGetScalar(args[0]);
  }
  else if (args.size() == 2 && mxIsDouble(args[0]) && mxIsDouble(args[1])) {
    id_ = mxGetScalar(args[0]);
    timeout_ = mxGetScalar(args[1]);
  }
  else
    ERROR("Invalid argument.");
}

void TimeoutOperation::run(int nlhs, mxArray* plhs[]) {
  if (nlhs)
    ERROR("Too many output: %d for 1.", nlhs);
  sqlite3mex::Database* connection = manager()->get(id_);
  if (!connection->busy_timeout(timeout_))
    ERROR("Failed to set timeout.");
}

} // namespace sqlite3mex
