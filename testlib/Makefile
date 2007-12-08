include ../Makefile.inc
LDFLAGS=$(oslibflags)
LIBS=-ldl -lpthread

OBJECTS=testlib.o
TARGET=../libtestlib.$(libext)

all: $(TARGET)

$(TARGET): $(OBJECTS) Makefile
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS) $(OBJECTS)

testlib.o: testlib.c testlib.h Makefile

clean:
	-rm *.o

.PHONY: all clean
