HEADERS=$(wildcard lev_impl/*.hh)
TEST_SRCS=$(wildcard test/*.cc)
TESTS=$(TEST_SRCS:.cc=)
CFLAGS=-DNDEBUG -Wall -std=c++11 -I. -O2 -fno-rtti -fno-exceptions
CC:=g++

all: $(TESTS)

$(TESTS): $(HEADERS)

.cc:
	$(CC) $(CFLAGS) $*.cc $(TARGET) -o $*

clean:
	rm -f $(OBJS) $(TESTS) $(TARGET)

