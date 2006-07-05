/*
 * $Id: xd.c,v 1.6 2006/07/05 14:59:53 urs Exp $
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define BSIZE 4096

static void dump_file(char *fname);
static ssize_t rread(int fd, void *buffer, size_t count);
static void dump(char *dst, void *src, int len, void **lastp, int *flagp,
		 int addr);
static void address(char *dst, int addr, char term);

int main(int argc, char **argv)
{
    if (argc > 1)
	while (++argv, --argc)
	    dump_file(*argv);
    else
	dump_file("-");

    return 0;
}

static void dump_file(char *fname)
{
    unsigned char buffer1[BSIZE], buffer2[BSIZE];
    unsigned char *buffer;
    void *last = NULL;
    char out[5 * BSIZE];
    int addr = 0;
    int fd, nbytes, flag = 0;

    if (strcmp(fname, "-") == 0)
	fd = 0;
    else if ((fd = open(fname, O_RDONLY)) < 0) {
	perror(fname);
	return;
    }

    buffer = buffer1;
    while ((nbytes = rread(fd, buffer, BSIZE)) > 0) {
	dump(out, buffer, nbytes, &last, &flag, addr);
	fputs(out, stdout);
	addr += nbytes;
	buffer = (buffer == buffer1) ? buffer2 : buffer1;
    }
    address(out, addr, '\n');
    fputs(out, stdout);

    if (nbytes < 0)
	perror(fname);
    if (fd != 0)
	close(fd);
}

static ssize_t rread(int fd, void *buffer, size_t count)
{
    int ret = 0, nbytes;
    char *buf = buffer;

    while ((nbytes = read(fd, buf, count)) > 0)
	count -= nbytes, buf += nbytes, ret += nbytes;
    if (ret == 0)
	ret = nbytes;

    return ret;
}

#define HEX(n, i) ("0123456789abcdef"[((n) >> 4 * i) & 0xf])

static void dump(char *dst, void *src, int len, void **lastp, int *flagp,
		 int addr)
{
    unsigned char *buffer = src, *last;
    unsigned char *ptr;
    char *cp = dst;
    int count, ident, flag, i;

    last = *lastp;
    flag = *flagp;

    for (ptr = buffer; ptr < buffer + len; ptr += 16) {
	count = len - (ptr - buffer);
	if (count >= 16) {
	    count = 16;
	    ident = last && memcmp(last, ptr, 16) == 0;
	} else
	    ident = 0;

	if (!ident) {
	    address(cp, addr, ' ');
	    cp += 7;
	    for (i = 0; i < 16; i++) {
		if (i % 4 == 0)
		    *cp++ = ' ';
		if (i < count)
		    *cp++ = HEX(ptr[i], 1), *cp++ = HEX(ptr[i], 0);
		else
		    *cp++ = ' ', *cp++ = ' ';
	    }
	    *cp++ = ' ', *cp++ = ' ';
	    for (i = 0; i < count; i++)
		*cp++ = isprint(ptr[i]) ? ptr[i] : '.';
	    *cp++ = '\n';
	    flag = 0;
	} else if (!flag) {
	    *cp++ = '*', *cp++ = '\n';
	    flag = 1;
	}
	addr += count;
	last = ptr;
    }
    *cp = 0;

    *lastp = last;
    *flagp = flag;
}

static void address(char *dst, int addr, char term)
{
    *dst++ = HEX(addr, 5);
    *dst++ = HEX(addr, 4);
    *dst++ = HEX(addr, 3);
    *dst++ = HEX(addr, 2);
    *dst++ = HEX(addr, 1);
    *dst++ = HEX(addr, 0);
    *dst++ = term;
    *dst = 0;
}
