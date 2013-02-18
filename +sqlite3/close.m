function close( varargin )
%CLOSE Close a database.
%
%    sqlite3.close()
%    sqlite3.close(db_id)
%
% The close operation closes the connection to the database specified by
% the connection id `db_id`. When `db_id` is omitted, the default connection is
% closed.
%
% See also sqlite3.open sqlite3.execute
    mex_function_('close', varargin{:});
end

