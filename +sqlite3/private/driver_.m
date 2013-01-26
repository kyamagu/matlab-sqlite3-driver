%DRIVER_ Mex driver for sqlite3 database.
%
% Usage:
% 
%     driver_([operation,] [db,] [argment1, argument2, ...])
%
% The first argument operation specifies what operation to apply to the
% database connection. There are 4 kinds of operations.
%
%     open      Open a database.
%     close     Close a database connection.
%     execute   Execute an SQLite statement.
%     timeout   Set timeout value when databse is busy.
%
% The second argument db is the connection id. When omitted, default
% connection is used, which is the first database already opened.
%
% See the following for the detailed specification of the argument syntax
% for each operation.
% 
% __open__
% 
% Syntax:
% 
%     db = driver_('open', filename)
% 
% The open operation takes a file name of the database and returns newly
% created connection id. This id can be used for this connection until
% closed.
% 
% Example:
% 
%     >> db = driver_('open', '/path/to/test.db')
% 
% 
% __close__
% 
% Syntax:
% 
%     driver_('close', db)
%     driver_('close')
% 
% The close operation closes the connection to the database specified by
% the connection id `db`. When `db` is omitted, default connection is
% closed.
% 
% __execute__
% 
% Syntax:
% 
%     result = driver_('execute', db, sql, param1, param2, ...)
%     result = driver_('execute', sql, param1, param2, ...)
% 
% The execute operation apply sql staement `sql` in the database specified
% by the connection id `db`. When `db` is omitted, default connection is
% used.
% 
% The sql statement can use binding of the value through `?` as the
% placeholder. When the binding is used, there must be the corresponding
% number of parameters followed by the sql statement.
% 
% Example:
% 
%     >> result = driver_('execute', db, 'SELECT * FROM table')
%     >> result = driver_('execute', 'SELECT * FROM table WHERE foo = ?', 'bar')
% 
% __timeout__
% 
% Syntax:
% 
%     driver_('timeout', db, millisecond)
%     driver_('timeout', millisecond)
% 
% The timeout operation sets how long the driver should wait when the
% database is locked by other processes.
% 
% Example:
% 
%     >> driver_('timeout', db, 1000)
%     >> driver_('timeout', 1000)