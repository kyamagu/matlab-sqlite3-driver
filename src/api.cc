// SQLite3 matlab driver main.
//
// Kota Yamaguchi 2012 <kyamagu@cs.stonybrook.edu>

#include <limits>
#include <mexplus.h>
#include <sqlite3mex.h>

using namespace mexplus;
using namespace sqlite3mex;

template class mexplus::Session<Database>;

namespace {

intptr_t getDefaultId() {
  intptr_t id = 0;
  const Session<Database>::InstanceMap& map =
      Session<Database>::getInstanceMap();
  if (!map.empty())
    id = map.rbegin()->first;
  return id;
}

MEX_DEFINE(open) (int nlhs, mxArray* plhs[],
                  int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 1, 8, "ReadOnly", "ReadWrite", "Create",
      "NoMutex", "FullMutex", "SharedCache", "PrivateCache", "OpenURI");
  OutputArguments output(nlhs, plhs, 1);
  string filename(input.get<string>(0));
  int flags =
      ((input.get<bool>("ReadOnly", false)) ? SQLITE_OPEN_READONLY : 0) |
      ((input.get<bool>("ReadWrite", nrhs == 1)) ? SQLITE_OPEN_READWRITE : 0) |
      ((input.get<bool>("Create", nrhs == 1)) ? SQLITE_OPEN_CREATE : 0) |
      ((input.get<bool>("NoMutex", false)) ? SQLITE_OPEN_NOMUTEX : 0) |
      ((input.get<bool>("FullMutex", false)) ? SQLITE_OPEN_FULLMUTEX : 0) |
      ((input.get<bool>("SharedCache", false)) ? SQLITE_OPEN_SHAREDCACHE : 0) |
      ((input.get<bool>("PrivateCache", false)) ?
          SQLITE_OPEN_PRIVATECACHE : 0) |
      ((input.get<bool>("OpenURI", false)) ? SQLITE_OPEN_URI : 0);
  unique_ptr<Database> database(new Database());
  if (!database->open(filename, flags))
    ERROR(database->errorMessage());
  output.set(0, Session<Database>::create(database.release()));
}

MEX_DEFINE(close) (int nlhs, mxArray* plhs[],
                   int nrhs, const mxArray* prhs[]) {
  InputArguments input;
  input.define("default", 0);
  input.define("id-given", 1);
  input.parse(nrhs, prhs);
  OutputArguments output(nlhs, plhs, 0);
  intptr_t id = (input.is("id-given")) ?
      input.get<intptr_t>(0) : getDefaultId();
  if (id > 0)
    Session<Database>::destroy(id);
}

MEX_DEFINE(execute) (int nlhs, mxArray* plhs[],
                     int nrhs, const mxArray* prhs[]) {
  if (nrhs < 1)
    ERROR("Too few input arguments: %d for at least %d.", nrhs, 1);
  if (nlhs > 1)
    ERROR("Too many output arguments: %d for at most %d.", nlhs, 1);
  vector<const mxArray*> rhs(prhs, prhs + nrhs);
  vector<const mxArray*>::iterator it = rhs.begin();
  intptr_t id = (mxIsNumeric(*it)) ?
      MxArray::to<intptr_t>(*(it++)) : getDefaultId();
  string sql = MxArray::to<string>(*(it++));
  vector<const mxArray*> params(it, rhs.end());
  Database* database = Session<Database>::get(id);
  if (!database->execute(sql, params, &plhs[0]))
    ERROR("%s: %s", database->errorMessage(), sql.c_str());
}

MEX_DEFINE(timeout) (int nlhs, mxArray* plhs[],
                     int nrhs, const mxArray* prhs[]) {
  InputArguments input;
  input.define("default", 1);
  input.define("id-given", 2);
  input.parse(nrhs, prhs);
  OutputArguments output(nlhs, plhs, 0);
  intptr_t id;
  int timeout;
  if (input.is("default")) {
    id = getDefaultId();
    timeout = input.get<int>(0);
  }
  else {
    id = input.get<intptr_t>(0);
    timeout = input.get<int>(1);
  }
  Database* database = Session<Database>::get(id);
  if (!database->busyTimeout(timeout))
    ERROR("Failed to set timeout.");
}

} // namespace

MEX_DISPATCH
