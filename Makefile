#
# $Id: Makefile,v 1.1 2011/03/21 12:02:40 urs Exp $
#

INSTALL = /usr/bin/install
RM      = rm -f

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
	$(RM) *.o core xd
