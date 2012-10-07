function make(varargin)
%SQLITE3.MAKE  build a driver mex file
%
%    sqlite3.make(['optionName', optionValue, ...,] [compiler_flags])
%
% sqlite3.make builds a mex file for the sqlite3 driver. The make script
% accepts two options and compiler flags for the mex command. See below
% for the supported options.
%
% In addition to the options, sqlite3.make takes compiler options passed
% to mex command.
%
% Options:
%
%     Option name            Value
%     ---------------------  ---------------------------
%     --libsqlite3_path      path to libsqlite3.a
%     --libboost_regex_path  path to libboost_regex-mt.a
%
% By default, sqlite3.make builds a dynamically linked mex file. The above
% two options allow you to create a statistically linked mex file instead.
% Since matlab includes its own boost library which usually differs from
% system library, dynamically linked mex file might not work without
% library preloading.
%
% Example:
%
% The following assumes dependent libraries are located at /opt/local.
% Replace the path to where sqlite3, boost is installed in the system.
%
% Static linking
%
% >> sqlite3.make('--libsqlite3_path',     '/opt/local/lib/libsqlite3.a', ...
%                 '--libboost_regex_path', '/opt/local/lib/libboost_regex-mt.a', ...
%                 '-I/opt/local/include');
%
% Dynamic linking (not recommended)
%
% >> sqlite3.make('-I/opt/local/include', '-L/opt/local/lib');
%
% See also mex

    package_dir = fileparts(mfilename('fullpath'));
    try
        [sqlite3_path, boost_regex_path, compiler_flags] = ...
            parse_options(varargin{:});
        cmd = sprintf([...
            'mex -largeArrayDims %s %s -outdir %s -output driver %s %s %s'],...
            fullfile(package_dir, 'driver.cc'),...
            fullfile(package_dir, 'sqlite3mex.cc'),...
            package_dir,...
            sqlite3_path,...
            boost_regex_path,...
            strjoin(compiler_flags) ...
            );
        disp(cmd);
        eval(cmd);
    catch e
        disp(e.getReport);
    end

end

function [sqlite3_path, boost_regex_path, compiler_flags] = ...
    parse_options(varargin)
% parse boost_regex-mt and libsqlite3 options

    boost_regex_path = '-lboost_regex-mt';
    sqlite3_path = '-lsqlite3';
    mark_for_delete = false(size(varargin));
    for i = 1:2:numel(varargin)
        if strcmp(varargin{i}, '--libboost_regex_path')
            boost_regex_path = varargin{i+1};
            mark_for_delete(i:i+1) = true;
        end
        if strcmp(varargin{i}, '--libsqlite3_path')
            sqlite3_path = varargin{i+1};
            mark_for_delete(i:i+1) = true;
        end
    end
    compiler_flags = varargin(~mark_for_delete);

end

function str = strjoin(compiler_flags)
% concat array into string

    str = cellfun(@(s)[s, ' '],compiler_flags,'UniformOutput',false);
    str = [str{:}];

end
