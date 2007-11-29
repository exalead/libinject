CC=gcc
mode=develop
bits=64
ifeq ($(mode),develop)
CFLAGS=-O2 -g -fstrict-aliasing -Wall -Wextra -Werror -Wchar-subscripts -Wundef -Wshadow -Wcast-align -Wwrite-strings -Wsign-compare -Wunused -Wno-unused-parameter -Wuninitialized -Winit-self -Wpointer-arith -Wredundant-decls -Wformat-nonliteral -Wno-format-zero-length -Wno-format-y2k -Wmissing-format-attribute -Wbad-function-cast -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wdeclaration-after-statement -fPIC -m$(bits)
else ifeq ($(mode),debug)
CFLAGS=-O0 -g -fPIC
else
CFLAGS=-O2 -fPIC
endif

# vim:set syntax=make:
