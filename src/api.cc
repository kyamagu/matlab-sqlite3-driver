// SQLite3 matlab driver main.
//
// Kota Yamaguchi 2012 <kyamagu@cs.stonybrook.edu>

#include <limits>
#include "mex/arguments.h"
#include "mex/function.h"
#include "mex/mxarray.h"
#include "mex/session.h"
#include "sqlite3mex.h"

using mex::CheckInputArguments;
using mex::CheckOutputArguments;
using mex::MxArray;
using mex::Session;
using sqlite3mex::Database;

template class mex::Session<Database>;

namespace {

// Get a flag value for the specified field.
inline int GetFlag(const mxArray* options, const char* field, int flag) {
  return (mxIsLogicalScalarTrue(mxGetField(options, 0, field))) ? flag : 0;
}

// Get flag for the open operation.
int GetOpenFlags(const mxArray* options) {
  if (!mxIsStruct(options))
    ERROR("Invalid option.");
  return GetFlag(options, "ReadOnly", SQLITE_OPEN_READONLY) |
         GetFlag(options, "ReadWrite", SQLITE_OPEN_READWRITE) |
         GetFlag(options, "Create", SQLITE_OPEN_CREATE) |
         GetFlag(options, "NoMutex", SQLITE_OPEN_NOMUTEX) |
         GetFlag(options, "FullMutex", SQLITE_OPEN_FULLMUTEX) |
         GetFlag(options, "SharedCache", SQLITE_OPEN_SHAREDCACHE) |
         GetFlag(options, "PrivateCache", SQLITE_OPEN_PRIVATECACHE) |
         GetFlag(options, "OpenURI", SQLITE_OPEN_URI);
}

MEX_FUNCTION(open) (int nlhs,
                    mxArray* plhs[],
                    int nrhs,
                    const mxArray* prhs[]) {
  CheckInputArguments(2, 2, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  string filename(MxArray(prhs[0]).toString());
  int flags = GetOpenFlags(prhs[1]);
  Database* database = NULL;
  int database_id = Session<Database>::create(&database);
  if (!database->open(filename, flags)) {
    const char* errorMessage = database->errorMessage();
    Session<Database>::destroy(database_id);
    ERROR(errorMessage);
  }
  plhs[0] = MxArray(database_id).getMutable();
}

MEX_FUNCTION(close) (int nlhs,
                     mxArray* plhs[],
                     int nrhs,
                     const mxArray* prhs[]) {
  CheckInputArguments(0, 1, nrhs);
  CheckOutputArguments(0, 0, nlhs);
  int database_id = (nrhs > 0) ? MxArray(prhs[0]).toInt() : 0;
  Session<Database>::destroy(database_id);
}

MEX_FUNCTION(execute) (int nlhs,
                       mxArray* plhs[],
                       int nrhs,
                       const mxArray* prhs[]) {
  CheckInputArguments(1, numeric_limits<int>::max(), nrhs);
  CheckOutputArguments(0, 1, nlhs);
  vector<const mxArray*> rhs(prhs, prhs + nrhs);
  vector<const mxArray*>::iterator it = rhs.begin();
  int id = (mxIsNumeric(*it)) ?
      MxArray(*(it++)).toInt() : 0;
  string sql = MxArray(*(it++)).toString();
  vector<const mxArray*> params(it, rhs.end());
  Database* database = Session<Database>::get(id);
  if (!database)
    ERROR("No open database found.");
  if (!database->execute(sql, params, &plhs[0]))
    ERROR("%s: %s", database->errorMessage(), sql.c_str());
}

MEX_FUNCTION(timeout) (int nlhs,
                       mxArray* plhs[],
                       int nrhs,
                       const mxArray* prhs[]) {
  CheckInputArguments(1, 2, nrhs);
  CheckOutputArguments(0, 0, nlhs);
  int id = (nrhs == 1) ? 0 : MxArray(prhs[0]).toInt();
  int timeout = MxArray(prhs[nrhs == 2]).toInt();
  Database* database = Session<Database>::get(id);
  if (!database)
    ERROR("No open database found.");
  if (!database->busyTimeout(timeout))
    ERROR("Failed to set timeout.");
}

} // namespace
