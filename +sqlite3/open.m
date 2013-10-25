function database = open( filename )
%OPEN Open a new database.
%
%    database = sqlite3.open(filename)
%
% The open operation takes a file name of the database and returns newly
% created connection id. This id can be used for this connection until
% closed.
%
% See also sqlite3.close sqlite3.execute
    database = libsqlite3_('open', filename);
end

