%SQLITE3.DRIVER  driver for sqlite3 database
%
% Usage:
% 
%     sqlite3.driver([db,] [operation,] [argment1, argument2, ...])
% 
% The first argument db is the connection id. When omitted, default
% connection is used, which is the first database already opened.
%
% The second argument operation specifies what operation to apply to the
% database connection. There are 4 kinds of operations.
%
%     open      Open a database.
%     close     Close a database connection.
%     execute   Execute an SQLite statement.
%     timeout   Set timeout value when databse is busy.
%
% See the following for the detailed specification of the argument syntax
% for each operation.
% 
% __open__
% 
% Syntax:
% 
%     [db] = sqlite3.driver('open', filename)
% 
% The open operation takes a file name of the database and returns newly
% created connection id. This id can be used for this connection until
% closed.
% 
% Example:
% 
%     >> db = sqlite3.driver('open', '/path/to/test.db')
% 
% 
% __close__
% 
% Syntax:
% 
%     sqlite3.driver(db, 'close')
%     sqlite3.driver('close')
% 
% The close operation closes the connection to the database specified by
% the connection id `db`. When `db` is omitted, default connection is
% closed.
% 
% Example:
% 
%     >> sqlite3.driver(db, 'close')
%     >> sqlite3.driver('close')
% 
% __execute__
% 
% Syntax:
% 
%     [result] = sqlite3.driver(db, 'execute', sql, param1, param2, ...)
%     [result] = sqlite3.driver(db, sql, param1, param2, ...)
%     [result] = sqlite3.driver('execute', sql, param1, param2, ...)
%     [result] = sqlite3.driver(sql, param1, param2, ...)
% 
% The execute operation apply sql staement `sql` in the database specified
% by the connection id `db`. When `db` is omitted, default connection is
% used. The `'execute'` argument is optional.
% 
% The sql statement can use binding of the value through `?` as the
% placeholder. When the binding is used, there must be the corresponding
% number of parameters followed by the sql statement.
% 
% Example:
% 
%     >> result = sqlite3.driver(db, 'execute', 'SELECT * FROM table')
%     >> result = sqlite3.driver(db, 'SELECT * FROM table')
%     >> result = sqlite3.driver('execute', 'SELECT * FROM table')
%     >> result = sqlite3.driver('SELECT * FROM table')
%     >> result = sqlite3.driver('SELECT * FROM table WHERE rowid = ?', 1)
%     >> result = sqlite3.driver('SELECT * FROM table WHERE name = ?', 'foo')
% 
% __timeout__
% 
% Syntax:
% 
%     sqlite3.driver(db, 'timeout', millisecond)
%     sqlite3.driver('timeout', millisecond)
% 
% The timeout operation sets how long the driver should wait when the
% database is locked by other processes.
% 
% Example:
% 
%     >> sqlite3.driver(db, 'timeout', 1000)
%     >> sqlite3.driver('timeout', 1000)