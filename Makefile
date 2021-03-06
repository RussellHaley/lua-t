# vim: ts=3 sw=3 st=3 sts=3 sta noet tw=80 list
#
# \file      Makefile
# \brief     Makefile for the lua-t library
# \author    tkieslich
# \copyright See Copyright notice at the end of t.h

LVER=5.3

T_LIB_DYN=t.so
T_LIB_STA=t.a

PREFIX=$(shell pkg-config --variable=prefix lua)
INCDIR=$(shell pkg-config --variable=includedir lua)
LDFLAGS=$(shell pkg-config --libs lua) -lcrypt
PLAT=linux
MYCFLAGS=" "
SRCDIR=$(CURDIR)/src

CC=clang
LD=clang

all: $(SRCDIR)/$(T_LIB_DYN)

$(SRCDIR)/$(T_LIB_DYN):
	$(MAKE) -C $(SRCDIR) CC=$(CC) LD=$(LD) \
		T_LIB_DYN="$(T_LIB_DYN)" T_LIB_STA="$(T_LIB_STA)" \
		LVER="$(LVER)" \
		MYCFLAGS="$(MYCFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		INCS="$(INCDIR)" \
		PREFIX="$(PREFIX)"

install: $(SRCDIR)/$(T_LIB_DYN)
	$(MAKE) -C $(SRCDIR) CC=$(CC) LD=$(LD) \
		T_LIB_DYN="$(T_LIB_DYN)" T_LIB_STA="$(T_LIB_STA)" \
		LVER=$(LVER) \
		MYCFLAGS=$(MYCFLAGS) \
		LDFLAGS="$(LDFLAGS)" \
		INCS=$(INCDIR) \
		PREFIX="$(PREFIX)" install

test: $(SRCDIR)
	$(MAKE) -C $(SRCDIR) CC=$(CC) LD=$(LD) \
		LVER=$(LVER) \
		MYCFLAGS=$(MYCFLAGS) \
		LDFLAGS="$(LDFLAGS)" \
		INCDIR=$(INCDIR) test

# echo config parameters
echo:
	$(MAKE) -C $(SRCDIR) -s echo
	@echo "PLAT= $(PLAT)"
	@echo "LVER= $(LVER)"
	@echo "PREFIX= $(PREFIX)"
	@echo "INCDIR= $(INCDIR)"
	@echo "LDFLAGS= $(LDFLAGS)"
	@echo "MYCFLAGS= $(MYCFLAGS)"

clean:
	$(MAKE) -C $(SRCDIR) \
		T_LIB_DYN="$(T_LIB_DYN)" T_LIB_STA="$(T_LIB_STA)" \
		clean
