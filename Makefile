
CC = clang++
CFLAGS = -Wall -Wextra -Werror -pedantic
LANG = -x c++ -std=c++17
LIB = -stdlib=libc++
OPT = -O3 -DNDEBUG
DBG = -O0 -g -D_DEBUG
INC = -I/usr/include/gtk-2.0/ -I/usr/include/glib-2.0/ \
      -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ \
      -I/usr/include/cairo/ -I/usr/include/pango-1.0/ \
      -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include/ \
      -I/usr/include/gdk-pixbuf-2.0/ -I/usr/include/atk-1.0/ 

LNK = -lgtk-x11-2.0 -lgdk-x11-2.0 -lpangocairo-1.0 -latk-1.0 -lcairo \
      -lgdk_pixbuf-2.0 -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lgobject-2.0 \
      -lglib-2.0 -lfontconfig -lfreetype

COMPILE = $(CC) $(LANG) $(CFLAGS) $(LIB) $(OPT) $(INC)
LINK = $(CC) $(LIB) $(OPT) $(LNK)

compile: fact.out

clear:
	rm bin/*.o *.out

bin/main.o: src/main.cpp src/graph.h
	$(COMPILE) -c -o bin/main.o src/main.cpp

bin/graph.o: src/graph.cpp src/graph.h
	$(COMPILE) -c -o bin/graph.o src/graph.cpp

fact.out: bin/main.o bin/graph.o
	$(LINK) -o fact.out bin/main.o bin/graph.o


