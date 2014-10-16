function database = open(filename, varargin)
%OPEN Open a new database.
%
%    database = sqlite3.open(filename, ...)
%
% The open operation takes a file name of the database and returns newly
% created connection id. This id can be used for this connection until
% closed.
%
% The function takes optional flags.
%
%    'ReadOnly'     The database is opened in read-only mode.
%    'ReadWrite'    The database is opened for reading and writing if possible,
%                   or reading only if the file is write protected by the
%                   operating system.
%    'Create'       The database is created if it does not already exist.
%    'NoMutex'      The database connection opens in the multi-thread threading
%                   mode.
%    'FullMutex'    The database connection opens in the serialized threading
%                   mode.
%    'SharedCache'  The database connection becomes eligible to use shared
%                   cache mode.
%    'PrivateCache' Prevent the database to participate in shared cache mode.
%
% If the flag is not any of the 'ReadOnly', 'ReadWrite', or 'ReadWrite' +
% 'Create' combination with others, the behavior is undefined. The default flag
% is 'ReadWrite' + 'Create'.
%
% Example
% -------
%
%    sqlite3.open('mydata.sqlite3');
%    sqlite3.open('mydata.sqlite3', 'ReadOnly');
%    sqlite3.open('mydata2.sqlite3', 'ReadWrite', 'Create');
%
% See also sqlite3.close sqlite3.execute
  database = libsqlite3_('open', filename, varargin{:});
end
