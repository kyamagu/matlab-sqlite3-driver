Matlab SQLite3 Driver
=====================

Matlab driver for SQLite3 database. Features include:

 * Simple and clean Matlab API.
 * Fast C++ MEX implementation.
 * Parameter binding for an SQL statement.
 * Multiple database connections.
 * Easy manipulation of query results as a struct array.

Here is a quick example.

    addpath('/path/to/matlab-sqlite3-driver');
    sqlite3.open('/path/to/database.sqlite3');
    sqlite3.execute('CREATE TABLE records (id INTEGER, name VARCHAR)');
    sqlite3.execute('INSERT INTO records VALUES (?, ?)', 1, 'foo');
    sqlite3.execute('INSERT INTO records VALUES (?, ?)', 2, 'bar');
    records = sqlite3.execute('SELECT * FROM records WHERE id < ?', 10);
    result = sqlite3.execute('SELECT COUNT(*) FROM records');
    sqlite3.close();

Prerequisites
-------------

This driver is built on the Matlab `mex` interface. The package requires a C++
compiler for `mex`. In Windows, a C++ compiler is available from Visual Studio
and Microsoft SDK. Check
[Mathworks webpage](http://www.mathworks.com/support/compilers) for a supported
compiler.

In Linux, a compiler is usually available in a package manager. For example,
in Debian/Ubuntu:

    $ apt-get install build-essential

In Mac OS, install XCode and also UNIX command line tools. Also check
[this page](http://www.mathworks.com/support/solutions/en/data/1-FR6LXJ/)
to apply patches if necessary.

After installing build tools, set up the `mex` command in Matlab.

    >> mex -setup

Build
-----

Add path to the `matlab-sqlite3-driver` first. Then, call `sqlite3.make` in
Matlab.

    >> sqlite3.make

Runtime requirement
-------------------

Always add path to this driver before use within Matlab.

    >> addpath /path/to/matlab-sqlite3-driver

In most cases, you'll need to force Matlab to preload the system library due
to the incompatibility between Matlab's internal C++ runtime. Use `LD_PRELOAD`
variable to do so. For example, in Ubuntu 12.04 LTS 64-bit,

    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libstdc++.so.6:/lib/x86_64-linux-gnu/libgcc_s.so.1 matlab

The required path depends on the OS. You can check which path to specify by
comparing the output of `ldd` tool in the UNIX shell and in Matlab.

From the UNIX shell,

    $ ldd +sqlite3/private/libsqlite3_.mex*

From Matlab shell,

    >> !ldd +sqlite3/private/libsqlite3_.mex*

And find a dependent library differing in Matlab. You must append that library
in the `LD_PRELOAD` path.

In OS X, `LD_PRELOAD` equivalent is `DYLD_INSERT_LIBRARIES`. Use `otool -L`
command instead of `ldd`.

Test
----

Optionally test the functionality with the attached test script.

    >> addpath test/
    >> test_sqlite3

API
---

There are 4 public functions. All functions are scoped under `sqlite3`
namespace. Also check `help` of each function.

    open      Open a database.
    close     Close a database connection.
    execute   Execute an SQLite statement.
    timeout   Set timeout value when database is busy.

__open__

    database = sqlite3.open(filename)

The open operation takes a file name of a database and returns newly created
connection id. This id can be used for operations until closed.

Example:

    >> database = sqlite3.open('/path/to/test.db');

__close__

    sqlite3.close(database)
    sqlite3.close()

The close operation closes connection to the database specified by the
connection id `database`. When `database` is omitted, the last opened
connection is closed.

__execute__

    results = sqlite3.execute(database, sql, param1, param2, ...)
    results = sqlite3.execute(sql, param1, param2, ...)

The execute operation applies a sql statement `sql` in a database specified
by the connection id `database`. When `database` is omitted, the last opened
connection is used.

The sql statement can bind parameters through `?` as a placeholder.
When binding is used, there must be corresponding number of parameters
following the sql statement. Bind values can be a numeric scalar value,
a string, a uint8 array for blob, or an empty array for null.

Results are returned as a struct array.

Example:

    >> results = sqlite3.execute(database, 'SELECT * FROM records');
    >> results = sqlite3.execute('SELECT * FROM records');
    >> results = sqlite3.execute('SELECT * FROM records WHERE rowid = ? OR name = ?', 1, 'foo');
    >> results = sqlite3.execute('INSERT INTO records VALUES (?)', 'bar');

Metadata can be retrieved from `sqlite_master` table or from `PRAGMA`
statement.

    >> tables = sqlite3.execute('SELECT * FROM sqlite_master WHERE type="table"');
    >> indices = sqlite3.execute('SELECT * FROM sqlite_master WHERE type="index"');
    >> columns = sqlite3.execute('PRAGMA TABLE_INFO(records)');

__timeout__

    sqlite3.timeout(database, millisecond)
    sqlite3.timeout(millisecond)

The timeout operation sets how long the driver should wait when the database
is locked by other processes.

Example:

    >> sqlite3.timeout(1000);

Tips
----

### Mass insertion

Use transaction to insert a lot of data.

    sqlite3.execute('BEGIN');
    for i = 1:numel(X)
        sqlite3.execute('INSERT INTO records (x) values (?)', X(i));
    end
    sqlite3.execute('COMMIT');

Or, convert an array to a statement string.

    values = sprintf('(%g),', X);
    sqlite3.execute(['INSERT INTO records (x) values ', values(1:end-1)]);

License
-------

The code may be redistributed under BSD 3-clause license.
