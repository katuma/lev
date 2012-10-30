SRCS=$(wildcard src/*.cpp) main.cpp
HEADERS=$(wildcard inc/*.h)
OBJS=$(SRCS:.cpp=.o)


main: $(OBJS)
	clang -lstdc++ $(OBJS) -o main

$(OBJS): $(SRCS) $(HEADERS)

%.o: %.cpp
	clang -Iinc -Wall -std=c++11 -Iinc -O3 -fno-rtti -fno-exceptions -c $< -o $@
clean:
	rm -f src/*.o

#obj.cpp: src/obj.h
