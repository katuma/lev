SRCS=$(wildcard impl/*.cpp)
HEADERS=$(wildcard incl/*.h)
OBJS=$(SRCS:.cpp=.o)
TARGET=lev.a
TEST_SRCS=$(wildcard test/*.cc)
TESTS=$(TEST_SRCS:.cc=)
CFLAGS=-DNDEBUG -Wall -std=c++11 -Iincl -O2 -fno-rtti -fno-exceptions
CC:=g++
AR:=ar

all: $(TARGET) $(TESTS)

$(TARGET): $(OBJS)
	$(AR) rcu $@ $(OBJS)

$(OBJS): $(SRCS) $(HEADERS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

# .cc -> compile & link, .cpp -> just compile invariant
.cc: $(TARGET)
	$(CC) $(CFLAGS) $*.cc $(TARGET) -o $*
clean:
	rm -f $(OBJS) $(TESTS) $(TARGET)

