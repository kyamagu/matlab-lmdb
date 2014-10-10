function make(command, varargin)
%MAKE Build MEX files.
%
% make [command [options ...]]
%
% Build MEX files in the project. Customize this file to change how to build
% a project. By default, the script builds an example in the project directory,
% `make clean` delete any built binary, and
%
% Example
%
% lmdb.make
% lmdb.make('test')
% lmdb.make('clean')
%
% lmdb.make('all', '-I/opt/local/include', '-L/opt/local/lib')
%
  if nargin < 1, command = 'all'; end
  root_dir = fileparts(fileparts(mfilename('fullpath')));
  switch command
    case 'all'
      system('make -C src/liblmdb');
      targets = getTarget(root_dir);
      arrayfun(@(target)buildTarget(target, varargin{:}), targets);
    case 'clean'
      clear mex;
      system('make -C src/liblmdb clean');
      targets = getTarget(root_dir);
      deleteTargets(targets);
    case 'test'
      addpath(root_dir);
      run(fullfile(root_dir, 'test', 'testLMDB.m'));
    otherwise
      targets = getTarget(root_dir);
      index = strcmp(strrep({targets.name}, [root_dir, filesep], ''), ...
                     strrep(command, root_dir, ''));
      if ~any(index)
        error('make:unknownTarget', 'No rule to make %s.', command);
      end
      buildTarget(targets(index), varargin{:});
  end
end

function target = getTarget(root_dir)
%GETTARGET Get a build target.
  target = struct( ...
    'name', fullfile(root_dir, '+lmdb', 'private', 'LMDB_'), ...
    'sources', {{ ...
      fullfile(root_dir, 'src', 'LMDB_.cc'), ...
      }}, ...
    'options', sprintf('-I''%s'' -I''%s'' ''%s''', ...
                       fullfile(root_dir, 'include'), ...
                       fullfile(root_dir, 'src', 'liblmdb'), ...
                       fullfile(root_dir, 'src', 'liblmdb', 'liblmdb.a')) ...
    );
end

function buildTarget(target, varargin)
%BUILDTARGET Build a single target.
  if skipBuild(target)
    return;
  end
  command = sprintf('mex%s -output ''%s'' %s%s', ...
                    sprintf(' ''%s''', target.sources{:}), ...
                    target.name, ...
                    target.options, ...
                    sprintf(' %s', varargin{:}));
  disp(command);
  eval(command);
end

function flag = skipBuild(target)
%SKIPBUILD Check if the target is built and up-to-date.
  output = [target.name, '.', mexext];
  if ~exist(output, 'file')
    flag = false;
    return;
  end
  output_metadata = dir(output);
  sources_metadata = cellfun(@dir, target.sources);
  flag = all(output_metadata.datenum > [sources_metadata.datenum]);
end

function deleteTargets(targets)
%DELETETARGETS Delete target binaries.
  binaries = strcat({targets.name}, '.', mexext);
  for i = 1:numel(binaries)
    if exist(binaries{i}, 'file')
      fprintf('delete ''%s''\n', binaries{i});
      delete(binaries{i});
    end
  end
end
