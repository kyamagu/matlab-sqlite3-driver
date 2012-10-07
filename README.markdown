Matlab SQLite3 Driver
=====================

Matlab driver for SQLite3 database.

Prerequisite
------------

This driver is designed for matlab under UNIX environment.

This driver depends on the following software packages. Make sure these are
installed in the system.

 * C++ compiler for mex
 * Boost C++ Library
 * SQLite3

Compile
-------

The package comes with sqlite3.make script for compiling. See
`help sqlite3.make` for the detailed usage of the script.

The script can choose to build either statically linked binary or dynamically
linked binary. In general it is recommended to make a statically linked binary
if possible. However, if the static version of sqlite3 or boost_regex library
is not available, use dynamic linking.

The following example assumes libraries are installed under `/opt/local`.

__Static linking__

Specify static library paths and optional compiler flags in the argument.

    >> sqlite3.make('--libsqlite3_path',     '/opt/local/lib/libsqlite3.a', ...
                    '--libboost_regex_path', '/opt/local/lib/libboost_regex-mt.a', ...
                    '-I/opt/local/include');

__Dynamic linking__

Specify optional compiler flags in the argument.

    >> sqlite3.make('-I/opt/local/include', '-L/opt/local/lib')

When using dynamically linked binary, you might need to preload the shared
library on launching matlab. This is because matlab depends on its internal
boost library which is usually incompatible with the system. Set the
`LD_PRELOAD` variable on Linux or `DYLD_INSERT_LIBRARIES` variable on Mac OS X
when starting matlab.

Linux:

    LD_PRELOAD=/usr/lib/libboost_regex-mt.so matlab

Mac OS X:

    DYLD_INSERT_LIBRARIES=/opt/local/lib/libboost_regex-mt.dylib matlab

Usage
-----

The mex function `sqlite3.driver` supports following operations.

    Operation Description
    --------- ---------------------------------------
    open      Open a database.
    close     Close a database connection.
    execute   Execute an SQLite statement.
    timeout   Set timeout value when databse is busy.

The function accepts following argument syntax.

    sqlite3.driver([db,] [operation,] [argment1, argument2, ...])

See the following for the detailed specification of the syntax.

__open__

    [db] = sqlite3.driver('open', filename)

The open operation takes a file name of the database and returns newly created
connection id. This id can be used for this connection until closed.

Example:

    >> db = sqlite3.driver('open', '/path/to/test.db')


__close__

    sqlite3.driver(db, 'close')
    sqlite3.driver('close')

The close operation closes the connection to the database specified by the
connection id `db`. When `db` is omitted, default connection is closed.

Example:

    >> sqlite3.driver(db, 'close')
    >> sqlite3.driver('close')

__execute__

    [result] = sqlite3.driver(db, 'execute', sql, param1, param2, ...)
    [result] = sqlite3.driver(db, sql, param1, param2, ...)
    [result] = sqlite3.driver('execute', sql, param1, param2, ...)
    [result] = sqlite3.driver(sql, param1, param2, ...)

The execute operation apply sql staement `sql` in the database specified by
the connection id `db`. When `db` is omitted, default connection is used.
The `'execute'` argument is optional.

The sql statement can use binding of the value through `?` as the placeholder.
When the binding is used, there must be the corresponding number of parameters
followed by the sql statement.

Results are returned as a struct array.

Example:

    >> result = sqlite3.driver(db, 'execute', 'SELECT * FROM table')
    >> result = sqlite3.driver(db, 'SELECT * FROM table')
    >> result = sqlite3.driver('execute', 'SELECT * FROM table')
    >> result = sqlite3.driver('SELECT * FROM table')
    >> result = sqlite3.driver('SELECT * FROM table WHERE rowid = ? OR name = ?', 1, 'foo')

__timeout__

    sqlite3.driver(db, 'timeout', millisecond)
    sqlite3.driver('timeout', millisecond)

The timeout operation sets how long the driver should wait when the database
is locked by other processes.

Example:

    >> sqlite3.driver(db, 'timeout', 1000)
    >> sqlite3.driver('timeout', 1000)

License
-------

The code may be redistributed under BSD 3-clause license.
