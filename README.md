Matlab LMDB
===========

Matlab LMDB wrapper for UNIX environment.

 * See [LMDB website](http://symas.com/mdb/).
 * The implementation is based on [mexplus](http://github.com/kyamagu/mexplus).

See also [matlab-leveldb](http://github.com/kyamagu/matlab-leveldb).

Build
-----

    addpath /path/to/matlab-lmdb;
    lmdb.make();
    lmdb.make('test');

Edit `Makefile` to customize the build process.

Example
-------

    % Open and close.
    database = lmdb.DB('./db');
    readonly_database = lmdb.DB('./db', 'RDONLY', true);
    clear readonly_database;

    % Read and write.
    database.put('key1', 'value1');
    database.put('key2', 'value2');
    value1 = database.get('key1');
    database.remove('key1');

    % Iterator.
    database.each(@(key, value) disp([key, ': ', value]));
    count = database.reduce(@(key, value, count) count + 1, 0);

TODO
----

 * Finer transaction API.
