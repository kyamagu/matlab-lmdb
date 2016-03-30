function datum = toEncodedDatum(encoded_image, label)
%TOENCODEDDATUM Convert encoded image binary to datum.
%
% label = 0;
% fid = fopen('cat.jpg', 'r');
% jpg_image = fread(fid, inf, 'uint8=>uint8');
% fclose(fid);
% datum = caffe_pb.toEncodedDatum(jpg_image, label);
%
% See also caffe_pb.toDatum caffe_pb.fromDatum
  datum = caffe_pb_(mfilename, encoded_image, label);
end
