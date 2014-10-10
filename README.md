Matlab LMDB
==============

Matlab LMDB wrapper.

The implementation is based on [mexplus](http://github.com/kyamagu/mexplus).

Prerequisite
------------

 * [LMDB](http://symas.com/mdb/)

You may install LMDB via a package manager, such as `apt-get`.

Build
-----

    addpath /path/to/matlab-lmdb;
    lmdb.make();

To specify optional build flags:

    lmdb.make('all', '-I/opt/local/include -L/opt/local/lib');

Run a test:

    lmdb.make('test');

Example
-------

    % Open and close.
    database = lmdb.DB('./db');
    clear database;

    % Read and write.
    database.put('key1', 'value1');
    database.put('key2', 'value2');
    value1 = database.get('key1');
    database.remove('key1');

    % Iterator.
    database.each(@(key, value) disp([key, ': ', value]));
    count = database.reduce(@(key, value, count) count + 1, 0);
