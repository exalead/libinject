mode   = develop
bits   = 64
prefix = $(HOME)
bindir = $(prefix)/bin
libdir = $(prefix)/lib
vimdir = $(HOME)/.vim/syntax/

SUBDIRS=src test testlib
CLEANSUBDIRS=$(addprefix clean-,$(SUBDIRS))
SCRIPTS=$(addprefix $(bindir)/inject,graph hexdump forgedump)

all: src

test: src testlib
$(SUBDIRS):
	make -C $@ mode=$(mode) bits=$(bits)

doc:
	doxygen Doxyfile > /dev/null

clean-all: clean clean-doc
clean: $(CLEANSUBDIRS)
	-rm *.o
	-rm *.so

$(CLEANSUBDIRS): clean-%:
	make -C $* clean

clean-doc:
	-rm -rf doc

install: install-bin install-lib
install-bin: $(bindir)/inject $(SCRIPTS)
install-lib: $(libdir)/libinject.so

$(bindir)/inject: tools/inject.in $(bindir)
	sed -e 's;@@LIBDIR@@;$(libdir);g' $< > $@
	chmod +x $@

$(bindir)/injectgraph: tools/graphlog.py $(bindir)
	cp $< $@

$(bindir)/injecthexdump: tools/readdump.py $(bindir)
	cp $< $@

$(bindir)/injectforgedump: tools/forgedump.py $(bindir)
	cp $< $@

$(libdir)/libinject.so: libinject.so $(libdir)
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
