# Makefile for matlab-lmdb
LMDB_DIR := src/liblmdb
MAKE := make
RM := rm
ECHO := echo
MATLABDIR ?= /usr/local/matlab
MATLAB := $(MATLABDIR)/bin/matlab
MEX := $(MATLABDIR)/bin/mex
MEXEXT := $(shell $(MATLABDIR)/bin/mexext)
MEXFLAGS := -Iinclude -I$(LMDB_DIR) CXXFLAGS="\$$CXXFLAGS -std=c++11"
TARGET := +lmdb/private/LMDB_.$(MEXEXT)

.PHONY: all test clean

all: $(TARGET)

$(TARGET): src/LMDB_.cc $(LMDB_DIR)/liblmdb.a
	$(MEX) -output $@ $< $(MEXFLAGS) $(LMDB_DIR)/liblmdb.a

$(LMDB_DIR)/liblmdb.a: $(LMDB_DIR)
	$(MAKE) -C $(LMDB_DIR)

test: $(TARGET)
	$(ECHO) "run test/testLMDB" | $(MATLAB) -nodisplay

clean:
	$(MAKE) -C $(LMDB_DIR) clean
	$(RM) $(TARGET)
