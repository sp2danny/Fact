
CC = clang++
CFLAGS = -Wall -Wextra -Werror -pedantic
LANG = -x c++ -std=c++17
LIB = -stdlib=libc++
OPT = -O3 -DNDEBUG
DBG = -O0 -g -D_DEBUG

COMPILE = $(CC) $(LANG) $(CFLAGS) $(LIB) $(DBG)
LINK = $(CC) $(LIB) $(DBG)

compile: fact.out

clear:
	rm bin/*.o *.out

bin/main.o: src/main.cpp
	$(COMPILE) -c -o bin/main.o src/main.cpp

fact.out: bin/main.o
	$(LINK) -o fact.out bin/main.o


