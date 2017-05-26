#
# $Id: Makefile,v 1.2 2017/05/26 14:58:44 urs Exp $
#

INSTALL = /usr/bin/install
RM      = rm -f

# For out-of-tree builds we may override the src dir on the 'make'
# command line and we add the build directory (current dir) to the
# CPP include path.
CPPFLAGS = -I.
srcdir   = .

CFLAGS  = -O3 -Wall -Wextra
LDFLAGS = -s
prefix  = /usr/local

.PHONY: all
all: xd

.PHONY: install
install: xd
	$(INSTALL) -m 755 xd   $(prefix)/bin
	$(INSTALL) -m 644 xd.1 $(prefix)/man/man1

.PHONY: clean
clean:
	$(RM) *.o core xd hextab.c

xd.o: hextab.c

hextab.c:
	awk -f $(srcdir)/mktab.awk > $@
