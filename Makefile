# Makefile for matlab-lmdb
LMDBDIR := src/liblmdb
ECHO := echo
MATLABDIR ?= /usr/local/matlab
MATLAB := $(MATLABDIR)/bin/matlab
MEX := $(MATLABDIR)/bin/mex
MEXEXT := $(shell $(MATLABDIR)/bin/mexext)
MEXFLAGS := -Iinclude -I$(LMDBDIR) CXXFLAGS="\$$CXXFLAGS -std=c++11"
CAFFEDIR ?= ../caffe
TARGET := +lmdb/private/LMDB_.$(MEXEXT) 
PROTOOBJ := $(CAFFEDIR)/build/src/caffe/proto/caffe.pb.o

.PHONY: all test clean

all: $(TARGET) caffe_pb

+lmdb/private/LMDB_.$(MEXEXT): src/LMDB_.cc $(LMDBDIR)/liblmdb.a
	$(MEX) -output $@ $< $(MEXFLAGS) $(LMDBDIR)/liblmdb.a

$(LMDBDIR)/liblmdb.a: $(LMDBDIR)
	$(MAKE) -C $(LMDBDIR)

caffe_pb: +caffe_pb/private/caffe_pb_.$(MEXEXT)

+caffe_pb/private/caffe_pb_.$(MEXEXT): src/caffe_pb_.cc $(PROTOOBJ)
	$(MEX) -output $@ $< $(MEXFLAGS) -I$(CAFFEDIR)/build/src/caffe/proto $(PROTOOBJ) -lprotobuf

$(CAFFEDIR)/build/src/caffe/proto/caffe.pb.o: $(CAFFEDIR)/src/caffe/proto/caffe.proto
	$(MAKE) -C $(CAFFEDIR) $(PROTOOBJ)

test: $(TARGET)
	$(ECHO) "run test/testLMDB" | $(MATLAB) -nodisplay
	$(ECHO) "run test/testCaffePB" | $(MATLAB) -nodisplay

clean:
	$(MAKE) -C $(LMDBDIR) clean
	$(RM) $(TARGET) +caffe_pb/private/caffe_pb_.$(MEXEXT)
