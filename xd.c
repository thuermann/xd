/*
 * $Id: xd.c,v 1.4 2006/05/03 20:48:28 urs Exp $
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define BSIZE 4096

static void dump_file(int fd);
static ssize_t rread(int fd, void *buffer, size_t count);
static void dump(char *dst, void *src, int len, void **lastp, int *flagp,
		 int address);

int main(int argc, char **argv)
{
    int fd;

    if (argc > 1)
	while (++argv, --argc) {
	    if ((fd = open(*argv, O_RDONLY)) < 0) {
		perror(*argv);
		continue;
	    }
	    dump_file(fd);
	    close(fd);
	}
    else
	dump_file(0);

    return 0;
}

#define HEX(n, i) ("0123456789abcdef"[((n) >> 4 * i) & 0xf])

static void dump_file(int fd)
{
    unsigned char buffer1[BSIZE], buffer2[BSIZE];
    unsigned char *buffer;
    void *last = NULL;
    char *cp, out[5 * BSIZE];
    int address = 0;
    int nbytes, flag = 0;

    buffer = buffer1;
    while ((nbytes = rread(fd, buffer, BSIZE)) > 0) {
	dump(out, buffer, nbytes, &last, &flag, address);
	fputs(out, stdout);
	address += nbytes;
	buffer = (buffer == buffer1) ? buffer2 : buffer1;
    }
    cp = out;
    *cp++ = HEX(address, 5);
    *cp++ = HEX(address, 4);
    *cp++ = HEX(address, 3);
    *cp++ = HEX(address, 2);
    *cp++ = HEX(address, 1);
    *cp++ = HEX(address, 0);
    *cp++ = '\n';
    *cp = 0;
    fputs(out, stdout);

    if (nbytes < 0)
	perror("read");
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

static void dump(char *dst, void *src, int len, void **lastp, int *flagp,
		 int address)
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
	    *cp++ = HEX(address, 5);
	    *cp++ = HEX(address, 4);
	    *cp++ = HEX(address, 3);
	    *cp++ = HEX(address, 2);
	    *cp++ = HEX(address, 1);
	    *cp++ = HEX(address, 0);
	    *cp++ = ' ';
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
	address += count;
	last = ptr;
    }
    *cp = 0;

    *lastp = last;
    *flagp = flag;
}
