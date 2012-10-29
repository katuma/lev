SRCS=$(wildcard src/*.cpp)
OBJS=$(SRCS:.cpp=.o)

all: $(OBJS)

%.o: %.cpp
	clang -DNDEBUG -Wall -std=c++11 -Iinc -O3 -fno-rtti -fno-exceptions -c $< -o $@
	#g++ -Wall -std=c++11 -Iinc -O2 -ggdb -c $< -o $@
clean:
	rm -f src/*.o

#obj.cpp: src/obj.h
