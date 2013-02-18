function make(varargin)
%MAKE Build a driver mex file.
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
        'mex -largeArrayDims%s -outdir %s -output mex_function_ %s%s'],...
        find_source_files(fullfile(package_dir, 'private')),...
        fullfile(package_dir, 'private'),...
        sqlite3_path,...
        compiler_flags...
        );
    disp(cmd);
    eval(cmd);

end

function [sqlite3_path, compiler_flags] = parse_options(varargin)
% Parse compiler options

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

function files = find_source_files(root_dir)
%SOURCE_FILES List of source files in a string.

    files = dir(root_dir);
    srcs = files(cellfun(@(x)~isempty(x), ...
                 regexp({files.name},'\S+\.(c)|(cc)|(cpp)|(C)')));
    srcs = cellfun(@(x)fullfile(root_dir, x), {srcs.name},...
                   'UniformOutput', false);
    subdirs = files([files.isdir] & cellfun(@(x)x(1)~='.',{files.name}));
    subdir_srcs = cellfun(@(x)find_source_files(fullfile(root_dir,x)),...
                          {subdirs.name}, 'UniformOutput', false);
    files = [sprintf(' %s', srcs{:}), subdir_srcs{:}];

end
