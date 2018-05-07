CC=g++-4.9
CFLAGS=-std=c++11 -c -g -Wall -fno-access-control
LDFLAGS=-lpthread
INCLUDES=-I../../ -I../../Test/UnitTest/gtest -I../../Test/UnitTest/gtest/include
SOURCEDIR=$(PWD)
TEST_OBJECTS=$(shell find $(SOURCEDIR)/../../Core $(SOURCEDIR)/../../DB $(SOURCEDIR)/../../Util -name '*_test.o')
GTEST_SOURCES=TestRunner.cc gtest/src/gtest-all.cc
GTEST_OBJECTS=$(GTEST_SOURCES:.cc=.o)
EXEC=test

all: $(GTEST_OBJECTS) $(GTEST_SOURCES) $(TEST_OBJECTS)
	$(CC) -o $(EXEC) $(GTEST_OBJECTS) $(TEST_OBJECTS) $(LDFLAGS)

%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@	
	
clean:
	$(RM) $(GTEST_OBJECTS) $(EXEC)

.DEFAULT_GOAL = all
