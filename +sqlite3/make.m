function make(command, varargin)
%MAKE Build a driver mex file.
  if nargin < 1
    command = 'all';
  end

  switch command
    case 'all'
      mex -c -Iinclude src/sqlite3/sqlite3.c -outdir src/sqlite3
      mex -Iinclude src/api.cc src/sqlite3mex.cc src/sqlite3/sqlite3.o ...
          -output +sqlite3/private/libsqlite3_
    case 'clean'
      delete src/sqlite3/sqlite3.o +sqlite3/private/libsqlite3_*
    otherwise
      error('Unknown command: %s', command);
  end
end
