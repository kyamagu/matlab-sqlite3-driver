function make(action, varargin)
%MAKE Build a driver mex file.
  if nargin < 1
    action = 'all';
  end

  switch action
    case 'all'
      options = '';
      if isunix() && ~ismac()
        options = ' -ldl -lboost_regex';
      end
      dispAndEval('mex -c -Iinclude src/sqlite3/sqlite3.c -outdir src/sqlite3');
      dispAndEval([...
          'mex -Iinclude src/api.cc src/sqlite3mex.cc src/sqlite3/sqlite3.%s ' ...
          '-output +sqlite3/private/libsqlite3_%s' ...
          ], ...
          libext, ...
          options);
    case 'clean'
      delete(sprintf('src/sqlite3/sqlite3.%s +sqlite3/private/libsqlite3_*', ...
                     libext));
    otherwise
      error('Unknown action: %s', action);
  end
end

function value = libext
%LIBEXT
  value = 'o';
  if ispc()
    value = 'obj';
  end
end

function dispAndEval(varargin)
%DISPANDEVAL
  command = sprintf(varargin{:});
  disp(command);
  eval(command);
end
