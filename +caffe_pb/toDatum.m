function datum = toDatum(image, label)
%TODATUM Convert image array to datum without encoding.
%
% label = 0;
% image = imread('cat.jpg');
% datum = caffe_pb.toDatum(image, label);
%
% See also caffe_pb.toEncodedDatum caffe_pb.fromDatum
  datum = caffe_pb_(mfilename, image, label);
end
