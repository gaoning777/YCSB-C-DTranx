CC=g++-4.9
CFLAGS=-std=c++11 -c -g -Wall -fno-access-control
INCLUDES=-I../../ -I../../Test/UnitTest/gtest -I../../Test/UnitTest/gtest/include
TEST_SOURCES=$(wildcard *_test.cc)
TEST_OBJECTS=$(TEST_SOURCES:.cc=.o)
DEP=$(TEST_SOURCES:.cc=.d)
-include $(DEP)

all: $(TEST_SOURCES) $(TEST_OBJECTS)

%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@
	$(CC) -MMD $(CFLAGS) $(INCLUDES) $*.cc > $*.d

clean:
	$(RM) $(TEST_OBJECTS) $(DEP)

.DEFAULT_GOAL = all