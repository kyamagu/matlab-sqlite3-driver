// Sqlite3 matlab driver main.
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

MEX_FUNCTION(open) (int nlhs,
                    mxArray* plhs[],
                    int nrhs,
                    const mxArray* prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  string filename(MxArray(prhs[0]).toString());
  Database* database = NULL;
  int database_id = Session<Database>::create(&database);
  if (!database->open(filename)) {
    const char* error_message = database->error_message();
    Session<Database>::destroy(database_id);
    ERROR(error_message);
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
    ERROR("%s: %s", database->error_message(), sql.c_str());
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
  if (!database->busy_timeout(timeout))
    ERROR("Failed to set timeout.");
}

} // namespace
