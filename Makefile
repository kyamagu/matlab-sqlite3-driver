# Makefile for matlab-sqlite3-driver
MATLABDIR ?= /usr/local/matlab
MATLAB := $(MATLABDIR)/bin/matlab
MEX := $(MATLABDIR)/bin/mex
MEXEXT := $(shell $(MATLABDIR)/bin/mexext)
MEXFLAGS := -Iinclude CXXFLAGS="\$$CXXFLAGS -std=c++11" -lboost_regex -ldl
SQLITE3DIR := src/sqlite3
TARGET := +sqlite3/private/libsqlite3_.$(MEXEXT)

.PHONY: all test clean

all: $(TARGET)

$(TARGET): src/api.cc src/sqlite3mex.cc $(SQLITE3DIR)/sqlite3.o
	$(MEX) -output $@ $^ $(MEXFLAGS)

$(SQLITE3DIR)/sqlite3.o:
	$(CC) -c -fPIC -o $@ -Iinclude $(SQLITE3DIR)/sqlite3.c

test: $(TARGET)
	echo "run test/testSQLite3" | $(MATLAB) -nodisplay

clean:
	$(RM) $(TARGET) $(SQLITE3DIR)/sqlite3.o
