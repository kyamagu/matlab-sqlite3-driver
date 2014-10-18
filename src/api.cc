// SQLite3 matlab driver main.
//
// Kota Yamaguchi 2012 <kyamagu@cs.stonybrook.edu>

#include <limits>
#include <mexplus.h>
#include <sqlite3mex.h>

using namespace mexplus;
using namespace sqlite3mex;

template class mexplus::Session<Database>;

namespace mexplus {

template <>
void MxArray::to(const mxArray* cell_array, vector<const mxArray*>* arrays) {
  if (!arrays)
    ERROR("Null pointer exception.");
  MxArray cell(cell_array);
  arrays->resize(cell.size());
  for (int i = 0; i < cell.size(); ++i)
    (*arrays)[i] = cell.at(i);
}

} // namespace mexplus

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
  InputArguments input;
  input.define("default", 2);
  input.define("id-given", 3);
  input.parse(nrhs, prhs);
  OutputArguments output(nlhs, plhs, 1);
  intptr_t id = 0;
  string sql;
  vector<const mxArray*> params;
  if (input.is("default")) {
    id = getDefaultId();
    input.get<string>(0, &sql);
    input.get<vector<const mxArray*> >(1, &params);
  }
  else {
    id = input.get<intptr_t>(0);
    input.get<string>(1, &sql);
    input.get<vector<const mxArray*> >(2, &params);
  }
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
  intptr_t id = 0;
  int timeout = 0;
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
