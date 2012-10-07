// Sqlite3 matlab driver.
//
// Kota Yamaguchi 2012 <kyamagu@cs.stonybrook.edu>

#include "sqlite3mex.h"

using sqlite3mex::Operation;

// Main entry point.
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  auto_ptr<Operation> operation(Operation::parse(nrhs, prhs));
  operation->run(nlhs, plhs);
}
