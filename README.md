Matlab LMDB
===========

Matlab LMDB wrapper for UNIX environment.

 * See [LMDB website](http://symas.com/mdb/).
 * The implementation is based on [mexplus](http://github.com/kyamagu/mexplus).

See also [matlab-leveldb](http://github.com/kyamagu/matlab-leveldb).

The package does not contain any data serialization. Use `char` for storing keys and values. You will need to serialize any Matlab variables to directly save in the database. For example, using `num2str` and `str2double`. There are different serialization options, such as [this serialization package](https://github.com/kyamagu/matlab-serialization) or [JSON format](https://github.com/kyamagu/matlab-json). Those using [Caffe](https://github.com/BVLC/caffe) might want to use a Datum converter in [the caffe-extension branch](https://github.com/kyamagu/matlab-lmdb/tree/caffe-extension).

Build
-----

Specify the MATLABDIR path.

    make MATLABDIR=/usr/local/MATLAB/R2015a
    make test

Edit `Makefile` to customize the build process.

In the caffe-extension branch, also specify the path to caffe.

    make MATLABDIR=/usr/local/MATLAB/R2015a CAFFEDIR=../caffe

Example
-------

    % Open and close.
    database = lmdb.DB('./db', 'MAPSIZE', 1024^3);
    clear database;
    readonly_database = lmdb.DB('./db', 'RDONLY', true, 'NOLOCK', true);
    clear readonly_database;

    % Read and write.
    database.put('key1', 'value1');
    database.put('key2', 'value2');
    value1 = database.get('key1');
    database.remove('key1');

    % Iterator.
    database.each(@(key, value) disp([key, ': ', value]));
    count = database.reduce(@(key, value, count) count + 1, 0);

    % Cursor.
    cursor = database.cursor('RDONLY', true);
    while cursor.next()
      key = cursor.key;
      value = cursor.value;
    end
    clear cursor;

    % Transaction.
    transaction = database.begin();
    try
      transaction.put('key1', 'value1');
      transaction.put('key2', 'value2');
      transaction.commit();
    catch exception
      transaction.abort();
    end
    clear transaction;

    % Dump.
    keys = database.keys();
    values = database.values();

See `help` documentation of each function, or visit [LMDB documentation](http://symas.com/mdb/doc/index.html) to understand the flags.

Caffe extension
---------------

The `caffe-extension` branch includes utilities to convert [`Datum` protocol buffer](https://github.com/BVLC/caffe/blob/master/src/caffe/proto/caffe.proto).

    % Make plain datum.
    image = imread('peppers.png');
    label = 1;
    datum = caffe_pb.toDatum(image, label);

    % Make encoded datum.
    fid = fopen('peppers.png', 'r');
    png_image = fread(fid, inf, 'uint8=>uint8');
    fclose(fid);
    label = 1;
    datum = caffe_pb.toEncodedDatum(png_image, label);

    % Read datum.
    [png_image, label] = caffe_pb.fromDatum(datum);

TODO
----

 * Finer transaction API.
