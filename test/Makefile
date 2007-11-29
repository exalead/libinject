include ../Makefile.inc

TESTS=parser config
TESTERS=$(addprefix test-,$(TESTS))

all: $(TESTERS)

$(TESTERS): test-%: % Makefile
	@LD_LIBRARY_PATH="../:./" LIBINJ_DISABLE=1 ./$<

$(TESTS): %: %.o Makefile
	gcc $(CFLAGS) -o $@ -L.. -linject -ltestlib $<

$(addsuffix .o,$(TESTS)): %.o: %.c ../testlib/testlib.h

clean:
	-rm *.o
	-rm $(TESTS)

.PHONY: $(TESTERS) all clean
