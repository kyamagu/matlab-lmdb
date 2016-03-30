function [image, label] = fromDatum(datum)
%FROMDATUM Convert datum to image and label.
%
% [image, label] = caffe_pb.fromDatum(datum);
%
% See also caffe_pb.toDatum caffe_pb.toEncodedDatum
  [image, label] = caffe_pb_(mfilename, datum);
end
