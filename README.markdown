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

Type `sqlite3.make` in matlab. If it gives an error, continue reading this
section.

The package comes with sqlite3.make script for compiling. The script can choose
to build either statically linked binary or dynamically linked binary. See
`help sqlite3.make` for the detailed usage of the script.

The following example assumes boost and sqlite3 are installed under
`/opt/local`. In a typical Linux environment, `sqlite3.make` doesn't need any
argument.

__Dynamic linking__

Specify any optional compiler flags in the argument.

    >> sqlite3.make('-I/opt/local/include', '-L/opt/local/lib')

__Static linking__

Specify static library paths and any optional compiler flags in the argument.

    >> sqlite3.make('--libsqlite3_path', '/opt/local/lib/libsqlite3.a', ...
                    '-I/opt/local/include');

Usage
-----

The mex function `sqlite3.driver` supports the following operations.

    Operation Description
    --------- ----------------------------------------
    open      Open a database.
    close     Close a database connection.
    execute   Execute an SQLite statement.
    timeout   Set timeout value when database is busy.

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
followed by the sql statement. Bind values can be a numeric scalar value,
string, uint8 array for blob, or empty array for null.

Results are returned as a struct array.

Example:

    >> result = sqlite3.driver(db, 'execute', 'SELECT * FROM records')
    >> result = sqlite3.driver(db, 'SELECT * FROM records')
    >> result = sqlite3.driver('execute', 'SELECT * FROM records')
    >> result = sqlite3.driver('SELECT * FROM records')
    >> result = sqlite3.driver('SELECT * FROM records WHERE rowid = ? OR name = ?', 1, 'foo')

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
