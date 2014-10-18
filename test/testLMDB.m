function testLMDB
%TESTLMDB Test the functionality of LMDB wrapper.

  addpath(fileparts(fileparts(mfilename('fullpath'))));
  % Using a database object.
  database = lmdb.DB('_testdb');
  value = database.get('some-key');
  assert(isempty(value));
  database.put('another-key', 'foo');
  database.put('yet-another-key', 'foo');
  database.remove('yet-another-key');
  value = database.get('another-key');
  assert(strcmp(value, 'foo'), 'ASSERTION FAILED: value = %s', value);
  % Iterator API.
  database.each(@(key, value) disp([key, ':', value]));
  foo_counter = @(key, value, accum) accum + strcmp(value, 'foo');
  foo_count = database.reduce(foo_counter, 0);
  fprintf('Number of ''foo'': %d\n', foo_count);
  clear database;
  database = lmdb.DB('_testdb');
  clear database;
  database = lmdb.DB('_testdb', 'RDONLY', true);
  clear database;
  if exist('_testdb', 'dir')
    rmdir('_testdb', 's');
  end
  fprintf('SUCCESS\n');

end
