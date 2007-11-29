include ../Makefile.inc
LDFLAGS=-shared
LIBS=-ldl -lpthread

OBJECTS=testlib.o

all: ../libtestlib.so

../libtestlib.so: $(OBJECTS) Makefile
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS) $(OBJECTS)

testlib.o: testlib.c testlib.h Makefile

clean:
	-rm *.o

.PHONY: all clean
