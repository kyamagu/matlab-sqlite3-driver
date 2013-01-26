function db_id = open( filename )
%SQLITE3.OPEN Open a new database.
%
%    db_id = sqlite3.open(filename)
%
% The open operation takes a file name of the database and returns newly
% created connection id. This id can be used for this connection until
% closed.
%
% See also sqlite3.close sqlite3.execute
    db_id = driver_('open', filename);
end

