function results = execute(varargin)
%EXECUTE Execute an SQL statement.
%
%     results = sqlite3.execute(sql, param1, param2, ...)
%     results = sqlite3.execute(database, sql, param1, param2, ...)
%
% The execute operation applies sql statement `sql` in the database specified
% by the connection id `db_id`. When `db_id` is omitted, the default connection
% is used.
%
% The sql statement can use binding of the value through `?` as the
% placeholder. When the binding is used, there must be the corresponding
% number of parameters followed by the sql statement.
%
% Example:
%     results = sqlite3.execute('SELECT * FROM records WHERE rowid = ?', 1)
%     results = sqlite3.execute(db_id, 'SELECT * FROM records WHERE name = ?', 'foo')
%
% See also sqlite3.open sqlite3.close
  nargchk(1, inf, nargin);
  if ischar(varargin{1})
    results = libsqlite3_('execute', varargin{1}, varargin(2:end));
  else
    nargchk(2, inf, nargin);
    results = libsqlite3_('execute', ...
                          varargin{1}, ...
                          varargin{2}, ...
                          varargin(3:end));
  end
end
