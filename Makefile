SRCS=$(wildcard impl/*.cpp)
HEADERS=$(wildcard incl/*.h)
OBJS=$(SRCS:.cpp=.o)
TEST_SRCS=$(wildcard test/*.cc)
TESTS=$(TEST_SRCS:.cc=)
CFLAGS=-DNDEBUG -Wall -std=c++11 -I. -O2 -fno-rtti -fno-exceptions
CC:=g++

all: $(TESTS)

$(OBJS): $(SRCS) $(HEADERS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

# .cc -> compile & link, .cpp -> just compile invariant
.cc:
	$(CC) $(CFLAGS) $*.cc $(TARGET) -o $*
clean:
	rm -f $(OBJS) $(TESTS) $(TARGET)

