function testCaffePB
%TESTCAFFEPB Test the functionality of Caffe PB utility.

  addpath(fileparts(fileparts(mfilename('fullpath'))));
  tests = { ...
    @test_to_and_from_datum, ...
    @test_to_and_from_encoded_datum ...
    };
  % Using a database object.
  fprintf('%s\n', mfilename);
  for i = 1:numel(tests)
    try
      fprintf('%s: ', func2str(tests{i}));
      feval(tests{i});
      fprintf('SUCCESS\n');
    catch exception
      fprintf('FAIL\n');
      disp(exception.getReport());
    end
  end
  fprintf('DONE\n');
end

function test_to_and_from_datum
  image = imread('peppers.png');
  label = 1;
  datum = caffe_pb.toDatum(image, label);
  [image2, label2] = caffe_pb.fromDatum(datum);
  assert(all(image(:) == image2(:)));
  assert(label == label2);
end

function test_to_and_from_encoded_datum
  fid = fopen('peppers.png', 'rb');
  image = fread(fid, inf, 'uint8=>uint8');
  fclose(fid);
  label = 1;
  datum = caffe_pb.toEncodedDatum(image, label);
  [image2, label2] = caffe_pb.fromDatum(datum);
  assert(all(image(:) == image2(:)));
  assert(label == label2);
end
