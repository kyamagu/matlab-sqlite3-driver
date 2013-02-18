// Sqlite3 matlab driver main.
//
// Kota Yamaguchi 2012 <kyamagu@cs.stonybrook.edu>

#include "mex/function.h"
#include "mex/mxarray.h"
#include "sqlite3mex.h"

using mex::CheckInputArguments;
using mex::CheckOutputArguments;
using mex::MxArray;
using sqlite3mex::Database;
using sqlite3mex::Session;

MEX_FUNCTION(open) (int nlhs,
                    mxArray* plhs[],
                    int nrhs,
                    const mxArray* prhs[]) {
  CheckInputArguments(1, 1, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  string filename(MxArray(prhs[0]).toString());
  plhs[0] = MxArray(Session::open(filename)).getMutable();
}

MEX_FUNCTION(close) (int nlhs,
                     mxArray* plhs[],
                     int nrhs,
                     const mxArray* prhs[]) {
  CheckInputArguments(0, 1, nrhs);
  CheckOutputArguments(0, 0, nlhs);
  int id = (nrhs == 1) ? MxArray(prhs[0]).toInt() : Session::default_id();
  Session::close(id);
}

MEX_FUNCTION(execute) (int nlhs,
                       mxArray* plhs[],
                       int nrhs,
                       const mxArray* prhs[]) {
  CheckInputArguments(1, 1024, nrhs);
  CheckOutputArguments(0, 1, nlhs);
  vector<const mxArray*> rhs(prhs, prhs + nrhs);
  vector<const mxArray*>::iterator it = rhs.begin();
  int id = (mxIsNumeric(*it)) ?
      MxArray(*(it++)).toInt() : Session::default_id();
  string sql = MxArray(*(it++)).toString();
  vector<const mxArray*> params(it, rhs.end());
  Database* connection = Session::get(id);
  if (!connection->execute(sql, params, &plhs[0]))
    ERROR("%s: %s", connection->error_message(), sql.c_str());
}

MEX_FUNCTION(timeout) (int nlhs,
                       mxArray* plhs[],
                       int nrhs,
                       const mxArray* prhs[]) {
  CheckInputArguments(1, 2, nrhs);
  CheckOutputArguments(0, 0, nlhs);
  int id = 0;
  int timeout = 0;
  if (nrhs == 1) {
    id = Session::default_id();
    timeout = MxArray(prhs[0]).toInt();
  }
  else {
    id = Session::default_id();
    timeout = MxArray(prhs[0]).toInt();
  }
  Database* connection = Session::get(id);
  if (!connection->busy_timeout(timeout))
    ERROR("Failed to set timeout.");
}