################################################
# Makefile for building cachebash              #
################################################


################################################
# Targets are:                                 #
# all - Builds cachebash                       #
# clean - Removes binary and build files       #
# test - build the gtest tests                 #
# run_all_tests - runs all the gtests          #
################################################

# Standard C++ compiler
CC = g++

GTEST_DIR = ../gtest-1.6.0/
GTEST_INCLUDE_DIR = ../gtest-1.6.0/include

# Include directories
I = -I ../ -I $(GTEST_INCLUDE_DIR)

# Compiler flags
CFLAGS = -Wall -levent -pthread -D_GNU_SOURCE $(I)
DFLAGS = -g

# Source files
SRC_DIR = .
SRC = cachebash.cc \
      config.cc \
      connection.cc \
      generator.cc \
      request.cc \
      response.cc \
      size_key_distribution.cc \
      statistic.cc \
      statistic_manager.cc \
      util.cc \
      warmup_sequence.cc \
      worker_manager.cc \
      worker_thread.cc

OBJ = $(patsubst %.cc, %.o, $(SRC))

# Tests
TESTS = request_test \
        size_key_distribution_test \
        statistic_test

# Google test directory
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# The loadtester binary
BINARY = cachebash

#Build rules

all: $(SRC)
	$(CC) -O3 $(CFLAGS) -o $(BINARY) $(SRC)

$(OBJ): $(SRC)
	$(CC) $(DFLAGS) $(CFLAGS) -c $(SRC)

debug: $(SRC)
	$(CC) $(CFLAGS) $(DFLAGS) -o $(BINARY) $(SRC)

test: $(OBJ) $(TESTS)

clean:
	rm -rf $(BINARY) *.o *.dSYM

# Build google test
gtest-all.o : $(GTEST_SRCS_)
	$(CC) $(CFLAGS) -I$(GTEST_DIR) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CC) $(CFLAGS) -I$(GTEST_DIR) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# Test build rules (Add per-test)
size_key_distribution_test.o : $(SRC_DIR)/size_key_distribution_test.cc \
                     $(SRC_DIR)/size_key_distribution.h $(GTEST_HEADERS)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/size_key_distribution_test.cc

size_key_distribution_test : util.o size_key_distribution.o size_key_distribution_test.o gtest_main.a
	$(CC) $(CFLAGS) -lpthread $^ -o $@

statistic_test.o : $(SRC_DIR)/statistic_test.cc \
                     $(SRC_DIR)/statistic.h $(GTEST_HEADERS)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/statistic_test.cc

statistic_test : util.o statistic.o statistic_test.o gtest_main.a
	$(CC) $(CFLAGS) -lpthread $^ -o $@

request_test.o : $(SRC_DIR)/request_test.cc \
                     $(SRC_DIR)/request.h $(GTEST_HEADERS)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/request_test.cc

request_test : util.o statistic.o request.o request_test.o gtest_main.a
	$(CC) -g $(CFLAGS) -lpthread $^ -o $@

# Runs all the tests
run_all_tests:
	for t in ${TESTS}; do \
		echo $$t; \
		./$$t; \
	done
