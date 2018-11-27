
CC = clang++
CFLAGS = -Wall -Wextra -Werror -pedantic
LANG = -x c++ -std=c++17
LIB = -stdlib=libc++
OPT = -O3 -DNDEBUG
DBG = -O0 -g -D_DEBUG

COMPILE = $(CC) $(LANG) $(CFLAGS) $(LIB) $(OPT)
LINK = $(CC) $(LIB) $(OPT)

compile: fact.out

clear:
	rm bin/*.o *.out

bin/main.o: src/main.cpp src/graph.h
	$(COMPILE) -c -o bin/main.o src/main.cpp

bin/graph.o: src/graph.cpp src/graph.h
	$(COMPILE) -c -o bin/graph.o src/graph.cpp

fact.out: bin/main.o bin/graph.o
	$(LINK) -o fact.out bin/main.o bin/graph.o


