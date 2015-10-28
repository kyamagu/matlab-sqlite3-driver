function testSQLite3
%TESTSQLITE3 Test the functionality of the sqlite3 driver.
  addpath(fileparts(fileparts(mfilename('fullpath'))));

  tests = {@test_functional_1, ...
           @test_functional_2, ...
           @test_functional_3};
  for i = 1:numel(tests)
    try
      tests{i}();
      fprintf('PASS: %s\n', func2str(tests{i}));
    catch e
      fprintf('FAIL: %s\n', func2str(tests{i}));
      fprintf('%s\n', e.getReport);
    end
  end

end

function test_functional_1
%TEST_FUNCTIONAL_1

  filename = fullfile(fileparts(mfilename('fullpath')), '_test_1_.sqlite3');
  function cleanup(filename)
    sqlite3.close();
    if exist(filename, 'file'), delete(filename); end
  end

  sqlite3.open(filename);
  try
    sqlite3.timeout(1000);
    sqlite3.execute('SELECT * FROM sqlite_master');
    s = sqlite3.execute('SELECT * FROM sqlite_master WHERE type = ?', 'table');
    assert(isempty(s));
  catch e
    cleanup(filename);
    rethrow(e);
  end
  cleanup(filename);

end

function test_functional_2
%TEST_FUNCTIONAL_2

  filename = fullfile(fileparts(mfilename('fullpath')), '_test_2_.sqlite3');
  function cleanup(db_id, filename)
    sqlite3.close(db_id);
    if exist(filename, 'file'), delete(filename); end
  end

  try
    db_id = sqlite3.open(filename);
    sqlite3.timeout(db_id, 1000);
    sqlite3.execute(db_id, 'SELECT * FROM sqlite_master');
    s = sqlite3.execute(db_id, ...
        'SELECT * FROM sqlite_master WHERE type = ?', ...
        'table');
    assert(isempty(s));
  catch e
    cleanup(db_id, filename);
    rethrow(e);
  end
  cleanup(db_id, filename);

end

function test_functional_3
%TEST_FUNCTIONAL_3
  fixture = struct(...
    'id', int32(1), ...
    'number', -2.4, ...
    'text', ['Here are unicodes: ', native2unicode([65, 192])], ...
    'binary', uint8(0:255) ...
    );
  filename = ':memory:';
  sqlite3.open(filename);
  sqlite3.execute([...
    'CREATE TABLE records(' ...
    'id INTEGER, ', ...
    'number REAL, ' ...
    'text TEXT, ' ...
    'binary BLOB' ...
    ')']);
  sqlite3.execute(...
    'INSERT INTO records(id, number, text, binary) VALUES (?,?,?,?)', ...
    fixture.id, fixture.number, fixture.text, fixture.binary);
  record = sqlite3.execute('SELECT * FROM records');
  assert(record.id == fixture.id);
  assert(record.number == fixture.number);
  assert(strcmp(record.text, fixture.text));
  assert(all(record.binary(:) == fixture.binary(:)));
  sqlite3.close();
end
