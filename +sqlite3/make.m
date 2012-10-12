function make(varargin)
%SQLITE3.MAKE  build a driver mex file
%
%    sqlite3.make(['optionName', optionValue,] [compiler_flags])
%
% sqlite3.make builds a mex file for the sqlite3 driver. The make script
% accepts sqlite3 path option and compiler flags for the mex command.
% See below for the supported build options.
%
% Options:
%
%    Option name          Value
%    -------------------  -------------------------------------------------
%    --libsqlite3_path    path to libsqlite3.a. e.g., /usr/lib/libsqlite3.a
%
% By default, sqlite3.make builds a dynamically linked mex file. The above
% option allows you to create a statistically linked mex file instead of
% dynamic linking.
%
% Example:
%
% The following example assumes dependent libraries are located at
% non-standard location /opt/local. Replace the path to where sqlite3 and
% boost is installed in the system.
%
% Dynamic linking
%
% >> sqlite3.make('-I/opt/local/include', '-L/opt/local/lib');
%
% Static linking
%
% >> sqlite3.make('--libsqlite3_path', '/opt/local/lib/libsqlite3.a', ...
%                 '-I/opt/local/include');
%
% See also mex

    package_dir = fileparts(mfilename('fullpath'));
    [sqlite3_path, compiler_flags] = parse_options(varargin{:});
    cmd = sprintf([...
        'mex -largeArrayDims %s %s -outdir %s -output driver %s%s'],...
        fullfile(package_dir, 'driver.cc'),...
        fullfile(package_dir, 'sqlite3mex.cc'),...
        package_dir,...
        sqlite3_path,...
        compiler_flags...
        );
    disp(cmd);
    eval(cmd);

end

function [sqlite3_path, compiler_flags] = parse_options(varargin)
% parse libsqlite3 options

    sqlite3_path = '-lsqlite3';
    mark_for_delete = false(size(varargin));
    for i = 1:2:numel(varargin)
        if strcmp(varargin{i}, '--libsqlite3_path')
            sqlite3_path = varargin{i+1};
            mark_for_delete(i:i+1) = true;
        end
    end
    compiler_flags = sprintf(' %s', varargin{~mark_for_delete});

end
