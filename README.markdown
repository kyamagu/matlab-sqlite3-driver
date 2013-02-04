Matlab SQLite3 Driver
=====================

Matlab driver for SQLite3 database. Features include:

 * Simple and clean Matlab API.
 * Fast C++ MEX implementation.
 * Parameter binding for an SQL statement.
 * Multiple database connections.
 * Easy manipulation of query results as a struct array.

Prerequisite
------------

This driver is designed for matlab under UNIX environment.

This driver depends on the following software packages. Make sure these are
installed in the system.

 * C++ compiler for mex
 * Boost C++ Library
 * SQLite3

In UNIX, these dependencies are usually available in the package manager. For
example, in Debian/Ubuntu, install packages by:

    $ apt-get install build-essential libsqlite3-dev libboost-dev

In macports, install packages by:

    $ port install sqlite3 boost

Compile
-------

Type `sqlite3.make` in matlab. This will compile the package with default
dependency to the installed libraries. If you need to specify where the
dependent libraries are installed, use add compiler flags to the `sqlite3.make`
script. Check the help of `sqlite3.make` for detail.

Example: Compile with the default library.

    >> sqlite3.make;

Example: Use libraries installed at `/opt/local`.

    >> sqlite3.make('-I/opt/local/include', '-L/opt/local/lib');

API
---

There are 4 public functions. All functions are scoped under `sqlite3`
namespace. Also check `help` of each function.

    open      Open a database.
    close     Close a database connection.
    execute   Execute an SQLite statement.
    timeout   Set timeout value when database is busy.

__open__

    db_id = sqlite3.open(filename)

The open operation takes a file name of the database and returns newly created
connection id. This id can be used for this connection until closed.

Example:

    >> db_id = sqlite3.open('/path/to/test.db');


__close__

    sqlite3.close(db_id)
    sqlite3.close()

The close operation closes the connection to the database specified by the
connection id `db_id`. When `db_id` is omitted, the default connection is
closed.

__execute__

    results = sqlite3.execute(db_id, sql, param1, param2, ...)
    results = sqlite3.execute(sql, param1, param2, ...)

The execute operation apply sql staement `sql` in the database specified by
the connection id `db_id`. When `db_id` is omitted, default connection is used.

The sql statement can bind values through `?` as a placeholder.
When binding is used, there must be corresponding number of parameters
following the sql statement. Bind values can be a numeric scalar value,
string, uint8 array for blob, or empty array for null.

Results are returned as a struct array.

Example:

    >> results = sqlite3.execute(db_id, 'SELECT * FROM records');
    >> results = sqlite3.execute('SELECT * FROM records');
    >> results = sqlite3.execute('SELECT * FROM records WHERE rowid = ? OR name = ?', 1, 'foo');
    >> results = sqlite3.execute('INSERT INTO records VALUES (?)', 'bar');

Meta-data can be retrieved from `sqlite_master` table or from `PRAGMA` statement.

    >> tables = sqlite3.execute('SELECT * FROM sqlite_master WHERE type="table"');
    >> indices = sqlite3.execute('SELECT * FROM sqlite_master WHERE type="index"');
    >> columns = sqlite3.execute('PRAGMA TABLE_INFO(records)');

__timeout__

    sqlite3.timeout(db_id, millisecond)
    sqlite3.timeout(millisecond)

The timeout operation sets how long the driver should wait when the database
is locked by other processes.

Example:

    >> sqlite3.timeout(1000);

License
-------

The code may be redistributed under BSD 3-clause license.
