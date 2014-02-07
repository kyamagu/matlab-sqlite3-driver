function timeout(varargin)
%TIMEOUT Set timeout value of the database connection.
%
%    sqlite3.timeout(database, millisecond)
%    sqlite3.timeout(millisecond)
%
% The timeout operation sets how long the driver should wait when the
% database is locked by other processes.
  libsqlite3_('timeout', varargin{:});
end
