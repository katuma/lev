SRCS=$(wildcard src/*.cpp)
OBJS=$(SRCS:.cpp=.o)

all: $(OBJS)

%.o: %.cpp
	g++ -std=c++11 -Iinc -O2 -ggdb -c $< -o $@
clean:
	rm -f src/*.o

#obj.cpp: src/obj.h
