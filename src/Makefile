include ../Makefile.inc
LDFLAGS=-shared
LIBS=-ldl -lpthread

SUBDIRS=actions conditions
CLEANSUBDIRS=$(addprefix clean-,$(SUBDIRS))
OBJECTS=binding.o ligHT.o socketinfo.o parser.o actions.o conffile.o runtime.o

all: $(SUBDIRS) ../libinject.so

../libinject.so: $(OBJECTS) actions/*.c conditions/*.c Makefile
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS) $(OBJECTS) actions/*.o conditions/*.o


binding.o: binding.c ligHT.h socketinfo.h actions.h conffile.h Makefile
ligHT.o: ligHT.c ligHT.h Makefile
socketinfo.o: socketinfo.c socketinfo.h Makefile
parser.o: parser.c parser.h Makefile
actions.o: actions.c actions.h socketinfo.h actionsdk.h parser.h actionlist.h conditionlist.h Makefile
conffile.o: conffile.c conffile.h actions.h parser.h Makefile
runtime.o: runtime.c runtime.h conffile.h Makefile

conditions actions: %: actionlist.h conditionlist.h
$(SUBDIRS):
	make -C $@ mode=$(mode) bits=$(bits)

actionlist.h conditionlist.h: %: .%
	@[ "$$(md5sum $@ | awk '{ print $$1 ; }')" = "$$(md5sum .$@ |  awk '{ print $$1 ; }')" ] || cp .$@ $@

.actionlist.h: actions/*.c ../scripts/buildActions.sh Makefile
	../scripts/buildActions.sh actions ActionTask > $@

.conditionlist.h: conditions/*.c ../scripts/buildActions.sh Makefile
	../scripts/buildActions.sh conditions ActionCondition > $@

clean: $(CLEANSUBDIRS)
	-rm conditionlist.h actionlist.h
	-rm .conditionlist.h .actionlist.h
	-rm *.o

$(CLEANSUBDIRS): clean-%:
	make -C $* clean

.PHONY: all $(SUBDIRS) clean $(CLEANSUBDIRS)
