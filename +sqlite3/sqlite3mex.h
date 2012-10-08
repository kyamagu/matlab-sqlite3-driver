// sqlite3 driver library.
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>
#include <deque>
#include <map>
#include <memory>
#include <mex.h>
#include <set>
#include <sqlite3.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

namespace sqlite3mex {

// Integer type.
typedef int64_t IntegerValue;
// Float type.
typedef double FloatValue;
// Text type.
typedef string TextValue;
// Blob value class.
typedef vector<uint8_t> BlobValue;
// Null value class.
typedef class _NullValue {} NullValue;
// Value of the sqlite row.
typedef boost::variant<IntegerValue, FloatValue, TextValue, BlobValue,
                       NullValue> Value;
// Column of the query result.
typedef struct {
  int type;            // Type of the column.
  string name;         // Name of the column.
  deque<Value> values; // Values of the column.
} Column;

// SQL statement object. It manages execution and query results.
class Statement {
public:
  // Create a new statement.
  Statement();
  // Finalize this statement.
  ~Statement();
  // Prepare the statement.
  bool prepare(const string& statement, sqlite3* database);
  // Finalize the statement.
  bool finalize();
  // Execute the prepared statement.
  bool step();
  // Reset the prepared statement.
  bool reset();
  // Bind parameters.
  bool bind(const vector<const mxArray*>& params);
  // Check if the return code is ok.
  bool ok() const;
  // Check if the return code is done.
  bool done() const;
  // Check if the return code is row.
  bool row() const;
  // Return the last result code.
  int code() const;
  // Return sqlite3_stmt* object.
  sqlite3_stmt* get();
  // Column count. The statement must be in ROW code. i.e., row() == true.
  int column_count() const;
  // Data count. The statement must be in ROW code. i.e., row() == true.
  int data_count() const;
  // Column name. The statement must be in ROW code. i.e., row() == true.
  string column_name(int i) const;
  // Column type. The statement must be in ROW code. i.e., row() == true.
  int column_type(int i) const;
  // Column value. The statement must be in ROW code. i.e., row() == true.
  // Caller is responsible for freeing (i.e., mxFree()) mxArray* after use.
  Value& column_value(int i);

private:
  // Prepared statement.
  sqlite3_stmt* statement_;
  // Return code.
  int code_;
  // Current value
  Value value_;
};

// Cache for statements.
class StatementCache {
public:
  typedef boost::unordered_map<string, Statement> CacheMap;
  static const int DEFAULT_CACHE_SIZE = 10;

  StatementCache(size_t cache_size = DEFAULT_CACHE_SIZE);
  ~StatementCache();

  // Get the cached statement.
  Statement& get(const string& statement, sqlite3* database);

private:
  // FIFO.
  deque<string> fifo_;
  // Lookup table.
  CacheMap table_;
  // Maximum cache size.
  size_t cache_size_;
};

// Database connection.
class Database {
public:
  Database();
  ~Database();
  // Open a connection.
  bool open(const string& filename);
  // Return the last error code.
  int error_code();
  // Return the last error message.
  string error_message();
  // Execute SQL statement.
  bool execute(const string& statement,
               const vector<const mxArray*>& params,
               mxArray** result);
  // Set timeout when busy.
  bool busy_timeout(int milliseconds);

private:
  // Close the connection.
  void close();
  // Fill in types and matlab-safe column names.
  void create_columns(const Statement& stmt, vector<Column>* columns);
  // Convert vector<Column> to mxArray*.
  bool convert_columns_to_array(vector<Column>* columns, mxArray** array);
  // Convert Value object to mxArray*.
  mxArray* convert_value_to_array(const Value& value, int type);

  // Statement cache.
  StatementCache statement_cache_;
  // SQLite3 C object.
  sqlite3* database_;
};

// Database session manager. Container for Database objects.
class DatabaseManager {
public:
  DatabaseManager();
  ~DatabaseManager();
  // Create a new connection.
  int open(const string& filename);
  // Close the connection.
  void close(int id);
  // Default id.
  int default_id() const;
  // Last id.
  int last_id() const;
  // Verify id.
  bool verify_id(int id) const;
  // Get the connection.
  Database* get(int id);

private:
  // Connection pool.
  map<int, Database> connections_;
  // Last id used.
  int last_id_;
};

// Abstract operation class. Child class must implement run() method.
class Operation {
public:
  // State definition for the parser.
  typedef enum {
    PARSER_INIT,
    PARSER_ID,
    PARSER_CMD,
    PARSER_FINISH
  } PARSER_STATE;

  // Destructor.
  virtual ~Operation() {}
  // Factory method for operation. Takes rhs of the mexFunction and return
  // a new operation object. Caller is responsible for the destruction after
  // use.
  static Operation* parse(int nrhs, const mxArray *prhs[]);
  // Execute the operation.
  virtual void run(int nlhs, mxArray* plhs[]) = 0;

protected:
  // Default constructor is prohibited. Use parse() method.
  Operation() : id_(0) {}
  // Accessor for the id.
  int id() const { return id_; }
  // Accessor for the operation arguments.
  const vector<const mxArray*>& arguments() { return args_; }
  // Accessor for the manager.
  static DatabaseManager* manager() { return &manager_; }

private:
  // Database session manager.
  static DatabaseManager manager_;
  // Connection id.
  int id_;
  // Arguments to the operation.
  vector<const mxArray*> args_;
};

// Open operation class.
class OpenOperation : public Operation {
public:
  // Open a connection.
  virtual void run(int nlhs, mxArray* plhs[]);
};

// Close operation class.
class CloseOperation : public Operation {
public:
  // Close the connection.
  virtual void run(int nlhs, mxArray* plhs[]);
};

// Execution operation class.
class ExecuteOperation : public Operation {
public:
  // Execute a statement.
  virtual void run(int nlhs, mxArray* plhs[]);
};

// Timeout operation class.
class TimeoutOperation : public Operation {
public:
  // Set or get timeout.
  virtual void run(int nlhs, mxArray* plhs[]);
};

} // namespace sqlite3mex
