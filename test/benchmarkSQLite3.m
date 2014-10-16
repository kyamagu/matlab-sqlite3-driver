function benchmarkSQLite3
%BENCHMARKSQLITE3 Benchmark the performance.
  tests = { ...
    @benchmark1 ...
    };
  for i = 1:numel(tests)
    try
      fprintf('%s\n', func2str(tests{i}));
      tests{i}();
    catch e
      fprintf('%s\n', e.getReport);
    end
  end
end

function time = measureTime(function_handle, trials)
  if nargin < 2
    trials = 1;
  end
  time = zeros(trials, 1);
  for i = 1:numel(trials)
    tic;
    feval(function_handle);
    time(i) = toc;
  end
  time = mean(time);
end

function benchmark1
%BENCHMARK_1
  sqlite3.open(':memory:');
  sqlite3.execute('CREATE TABLE records (FLOAT value)');
  fprintf('Parametric insertion: %g seconds.\n', ...
          measureTime(@benchmark1Parametric));
  sqlite3.execute('DELETE FROM records');
  fprintf('Non-parametric insertion: %g seconds.\n', ...
          measureTime(@benchmark1NonParametric));
  sqlite3.close();
end

function benchmark1Parametric
  X = rand(10000, 1);
  sqlite3.execute('BEGIN');
  for i = 1:numel(X)
    sqlite3.execute('INSERT INTO records VALUES (?)', X(i));
  end
  sqlite3.execute('END');
end

function benchmark1NonParametric
  X = rand(10000, 1);
  sqlite3.execute('BEGIN');
  for i = 1:numel(X)
    sqlite3.execute(sprintf('INSERT INTO records VALUES (%g)', X(i)));
  end
  sqlite3.execute('END');
end