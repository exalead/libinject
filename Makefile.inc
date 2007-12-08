CC   = gcc
mode = develop
os   = $(shell uname)
ifeq ($(os),SunOS)
oscflags=-std=c99 -D__EXTENSIONS__ -D_XPG4_2
osbinflags=-lsocket -lnsl -lrt
oslibflags=-shared
else
ifeq ($(os),Darwin)
oslibflags=-dynamiclib -flat_namespace -nostdlib -fno_common
oslibext=dylib
else
oslibflags=-shared
endif
endif
ifeq ($(mode),develop)
CFLAGS=-O2 -g -fstrict-aliasing -Wall -Wextra -Werror -Wchar-subscripts -Wundef \
       -Wshadow -Wcast-align -Wwrite-strings -Wsign-compare -Wunused            \
       -Wno-unused-parameter -Wuninitialized -Winit-self -Wpointer-arith        \
       -Wredundant-decls -Wformat-nonliteral -Wno-format-zero-length            \
       -Wno-format-y2k -Wmissing-format-attribute -Wbad-function-cast           \
       -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations          \
       -Wnested-externs -Wdeclaration-after-statement -fPIC                     \
       $(if $(bits),$(addprefix -m,$(bits))) $(oscflags)
else
ifeq ($(mode),debug)
CFLAGS=-O0 -g -fPIC $(if $(bits),$(addprefix -m,$(bits))) $(oscflags)
else
CFLAGS=-O2 -fPIC $(if $(bits),$(addprefix -m,$(bits))) $(oscflags)
endif
endif

# vim:set syntax=make:
