mode   = develop
prefix = $(HOME)
bindir = $(prefix)/bin
libdir = $(prefix)/lib
vimdir = $(HOME)/.vim/syntax/

SUBDIRS=src test testlib
CLEANSUBDIRS=$(addprefix clean-,$(SUBDIRS))
SCRIPTS=$(addprefix $(bindir)/inject,graph hexdump forgedump)

include Makefile.inc

all: src

test: src testlib
$(SUBDIRS):
	cd $@ && $(MAKE)

doc:
	doxygen Doxyfile > /dev/null

clean-all: clean clean-doc
clean: $(CLEANSUBDIRS)
	-rm *.o
	-rm *.so *.dylib

$(CLEANSUBDIRS): clean-%:
	cd $* && $(MAKE) clean

clean-doc:
	-rm -rf doc

install: install-bin install-lib
install-bin: $(bindir)/inject $(SCRIPTS)
install-lib: $(libdir)/libinject.$(libext)

ifeq ($(os),Darwin)
$(bindir)/inject: tools/inject.in $(bindir)
	sed -e 's;@@LIBDIR@@;$(libdir);g'	-e 's;@@LIBEXT@@;$(libext);g'					   \
			-e 's;LD_PRELOAD; DYLD_FORCE_FLAT_NAMESPACE=1  DYLD_INSERT_LIBRARIES;g'	 \
			$< > $@
	chmod +x $@
else
$(bindir)/inject: tools/inject.in $(bindir)
	sed -e 's;@@LIBDIR@@;$(libdir);g'  -e 's;@@LIBEXT@@;$(libext);g' $< > $@
	chmod +x $@
endif

$(bindir)/injectgraph: tools/graphlog.py $(bindir)
	cp $< $@

$(bindir)/injecthexdump: tools/readdump.py $(bindir)
	cp $< $@

$(bindir)/injectforgedump: tools/forgedump.py $(bindir)
	cp $< $@

$(libdir)/libinject.$(libext): libinject.$(libext) $(libdir)
	cp $< $@

install-vim-syntax: $(vimdir)/libinject.vim

$(vimdir)/libinject.vim: tools/libinject.vim $(vimdir)
	cp $< $@

$(bindir) $(libdir) $(vimdir):
	mkdir -p $@

uninstall:
	-rm $(bindir)/inject
	-rm $(bindir)/injectgraph
	-rm $(bindir)/injecthexdump
	-rm $(libdir)/libinject.so

q:
	@echo -e "Code statistics"
	@sloccount $(filter-out doc/, $(wildcard */)) 2> /dev/null | egrep '^[a-z]*:'

.PHONY: q install install-bin install-lib install-vim-syntax uninstall $(SUBDIRS) $(CLEANSUBDIRS) all doc clean-doc
