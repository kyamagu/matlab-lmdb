function testLMDB
%TESTLMDB Test the functionality of LMDB wrapper.

  addpath(fileparts(fileparts(mfilename('fullpath'))));
  % Using a database object.
  %test_readonly;
  %test_operations;
  test_transaction;
  if exist('_testdb', 'dir')
    rmdir('_testdb', 's');
  end
  fprintf('DONE\n');

end

function test_operations
  database = lmdb.DB('_testdb');
  value = database.get('some-key');
  assert(isempty(value));
  database.put('some-key', 'foo');
  database.put('another-key', 'bar');
  database.put('yet-another-key', 'baz');
  database.remove('yet-another-key');
  value = database.get('another-key');
  assert(strcmp(value, 'bar'), 'ASSERTION FAILED: value = %s', value);
  database.each(@(key, value) disp([key, ':', value]));
  foo_counter = @(key, value, accum) accum + strcmp(value, 'foo');
  foo_count = database.reduce(foo_counter, 0);
  assert(foo_count == 1, 'Expected %d, but observed %d\n', 1, foo_count);
end

function test_readonly
  database = lmdb.DB('_testdb');
  clear database;
  database = lmdb.DB('_testdb', 'RDONLY', true);
  error_raised = false;
  try
    database.put('key', 'value');
  catch
    error_raised = true;
  end
  assert(error_raised);
end

function test_transaction
  database = lmdb.DB('_testdb');
  transaction = database.begin();
  transaction.put('1', 'foo');
  transaction.put('2', 'bar');
  transaction.put('3', 'baz');
  transaction.abort();
  clear transaction;
  assert(isempty(database.get('1')));
  transaction = database.begin();
  transaction.put('1', 'foo');
  transaction.put('2', 'bar');
  transaction.put('3', 'baz');
  transaction.commit();
  assert(strcmp(database.get('1'), 'foo'));
  clear transaction;
  clear database; % Make sure database is not destroyed before transaction.
end
