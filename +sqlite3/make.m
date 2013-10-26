function make(varargin)
%MAKE Build a driver mex file.
%
%    sqlite3.make(['optionName', optionValue,] [compiler_flags])
%
% This function builds a mex file for the sqlite3 driver.
%
% Example:
%
%    sqlite3.make('-I/opt/local/include');
%
% See also mex
    package_dir = fileparts(mfilename('fullpath'));
    compiler_flags = parse_options(varargin{:});
    cmd = sprintf(...
        'mex -largeArrayDims -I%s -L%s%s -outdir %s -output libsqlite3_%s',...
        fullfile(fileparts(package_dir), 'include'),...
        fullfile(matlabroot, 'bin', computer('arch')),...
        find_source_files(fullfile(fileparts(package_dir), 'src')),...
        fullfile(package_dir, 'private'),...
        compiler_flags...
        );
    disp(cmd);
    eval(cmd);
end

function compiler_flags = parse_options(varargin)
% Parse compiler options.
    compiler_flags = sprintf(' %s', varargin{:});
    if isunix()
        compiler_flags = [compiler_flags, ' -ldl'];
    end
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
