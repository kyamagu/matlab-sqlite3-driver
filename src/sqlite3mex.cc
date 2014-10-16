// SQLite3 matlab driver library.
//
// Kota Yamaguchi 2012 <kyamagu@cs.stonybrook.edu>

#include <algorithm>
#if !__clang__ && (__GNUC__ < 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ < 8)))
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_replace;
#else
#include <regex>
#endif
#include <set>
#include <sqlite3mex.h>
#include <sstream>

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
    return false;
  for (int i = 0; i < params.size(); ++i) {
    if (mxGetNumberOfElements(params[i]) == 1 && mxIsNumeric(params[i])) {
      if (mxIsDouble(params[i]) || mxIsSingle(params[i]))
        code_ = sqlite3_bind_double(statement_, i + 1, mxGetScalar(params[i]));
      else
        code_ = sqlite3_bind_int64(statement_, i + 1, mxGetScalar(params[i]));
    }
    else if (mxIsChar(params[i])) {
      char* text = mxArrayToString(params[i]);
      code_ = sqlite3_bind_text(statement_,
                                i + 1,
                                text,
                                -1,
                                SQLITE_TRANSIENT);
      mxFree(text);
    }
    else if (mxIsUint8(params[i])) {
      code_ = sqlite3_bind_blob(statement_,
                                i + 1,
                                mxGetData(params[i]),
                                mxGetNumberOfElements(params[i]),
                                SQLITE_STATIC);
    }
    else if (mxIsEmpty(params[i]))
      code_ = sqlite3_bind_null(statement_, i + 1);
    else
      ERROR("Can't bind parameter %d.", i + 1);
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

int Statement::columnCount() const {
  return sqlite3_column_count(statement_);
}

int Statement::dataCount() const {
  return sqlite3_data_count(statement_);
}

const char* Statement::columnName(int i) const {
  return sqlite3_column_name(statement_, i);
}

int Statement::columnType(int i) const {
  return sqlite3_column_type(statement_, i);
}

const Value& Statement::columnValue(int i) {
  // It is possible to directly create an mxArray* here. However, due to the
  // memory allocation pattern in Matlab, it is faster to keep the result into
  // a temporary storage and convert the values to mxArray* after we find
  // the number of rows.
  switch (columnType(i)) {
    case SQLITE_INTEGER: {
      value_.first = SQLITE_INTEGER;
      value_.second = static_cast<int64_t>(sqlite3_column_int64(statement_, i));
      break;
    }
    case SQLITE_FLOAT: {
      value_.first = SQLITE_FLOAT;
      value_.second = sqlite3_column_double(statement_, i);
      break;
    }
    case SQLITE_TEXT: {
      const char* data = reinterpret_cast<const char*>(
          sqlite3_column_text(statement_, i));
      value_.first = SQLITE_TEXT;
      value_.second = string(data);
      break;
    }
    case SQLITE_BLOB: {
      const uint8_t* blob = reinterpret_cast<const uint8_t*>(
          sqlite3_column_blob(statement_, i));
      int bytes = sqlite3_column_bytes(statement_, i);
      value_.first = SQLITE_BLOB;
      value_.second = vector<uint8_t>(blob, blob + bytes);
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

StatementCache::StatementCache() :
    cache_size_(kDefaultCacheSize) {}

StatementCache::StatementCache(size_t cache_size) :
    cache_size_(cache_size) {}

StatementCache::~StatementCache() {}

Statement* StatementCache::get(const string& statement, sqlite3* database) {
  unordered_map<string, Statement>::iterator entry =
      table_.find(statement);
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

void StatementCache::clear() {
  table_.clear();
  fifo_.clear();
}

Database::Database() : database_(NULL) {}

Database::~Database() {
  close();
}

bool Database::open(const string& filename, int flags) {
  return sqlite3_open_v2(filename.c_str(),
                         &database_,
                         flags,
                         NULL) == SQLITE_OK;
}

void Database::close() {
  if (database_) {
    statement_cache_.clear();
    sqlite3_close(database_);
    database_ = NULL;
  }
}

int Database::errorCode() const {
  return sqlite3_errcode(database_);
}

const char* Database::errorMessage() const {
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
      createColumns(*statement, &columns);
      first_row = false;
    }
    for (int i = 0; i < statement->columnCount(); ++i)
      columns[i].values.push_back(statement->columnValue(i));
  }
  // TODO: check if the columns are valid.
  return statement->done() && convertColumnsToArray(&columns, result);
}

bool Database::busyTimeout(int milliseconds) {
  return sqlite3_busy_timeout(database_, milliseconds) == SQLITE_OK;
}

void Database::createColumns(const Statement& statement,
                             vector<Column>* columns) const {
  columns->resize(statement.columnCount());
  set<string> unique_names;
  for (int i = 0; i < statement.columnCount(); ++i) {
    // Clean the column name.
    static const regex leading_non_alphabets("^[^a-zA-Z]+");
    static const regex non_alphanumerics("[^a-zA-Z0-9]+");
    string name(statement.columnName(i));
    name = regex_replace(name, leading_non_alphabets, string(""));
    name = regex_replace(name, non_alphanumerics, string(" "));
    name.erase(0, name.find_first_not_of(' '));
    name.erase(name.find_last_not_of(' ') + 1);
    replace(name.begin(), name.end(), ' ', '_');
    transform(name.begin(), name.end(), name.begin(), ::tolower);
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

bool Database::convertColumnsToArray(vector<Column>* columns,
                                     mxArray** array) const {
  if (array == NULL)
    return false;
  if (columns->empty()) {
    *array = mxCreateStructMatrix(0, 0, 0, NULL);
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
        mxArray* element = convertValueToArray(values->front());
        mxSetFieldByNumber(*array, j++, i, element);
        values->pop_front();
      }
    }
  }
  return true;
}

mxArray* Database::convertValueToArray(const Value& value) const {
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
      // mxCreateString is simpler but slow for a long string.
      //array = mxCreateString(boost::get<TextValue>(value.second).c_str());
      const string& text = boost::get<TextValue>(value.second);
      mwSize dimensions[] = {1, static_cast<mwSize>(text.size())};
      array = mxCreateCharArray(2, dimensions);
      copy(text.begin(), text.end(), mxGetChars(array));
      break;
    }
    case SQLITE_BLOB: {
      const vector<uint8_t>& blob = boost::get<BlobValue>(value.second);
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
  if (array == NULL)
    ERROR("Failed to create mxArray.");
  return array;
}

} // namespace sqlite3mex
