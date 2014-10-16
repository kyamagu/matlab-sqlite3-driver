function make(varargin)
%MAKE Build a driver mex file.
  mex -v -c -Iinclude src/sqlite3/sqlite3.c -outdir src/sqlite3
  mex -v -Iinclude src/api.cc src/sqlite3mex.cc src/sqlite3/sqlite3.o ...
      src/mex/arguments.cc src/mex/mxarray.cc src/mex/function.cc ...
      -output +sqlite3/private/libsqlite3_
end
