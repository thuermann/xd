/*
 * $Id: xd.c,v 1.2 2005/02/17 13:51:18 urs Exp $
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define BSIZE 4096

static void dump_file(int fd);

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
    unsigned char *buffer, *last, *ptr;
    char *cp, line[80];
    unsigned int address = 0;
    int nbytes, count, i, flag, ident;

    buffer = buffer1;
    while ((nbytes = read(fd, buffer, BSIZE)) > 0) {
	for (ptr = buffer; ptr < buffer + nbytes; ptr += 16) {
	    count = nbytes - (ptr - buffer);
	    if (count >= 16) {
		count = 16;
		ident = address > 0 && memcmp(last, ptr, 16) == 0;
	    } else
		ident = 0;

	    if (!ident) {
		cp = line;
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
		    if (i < count) {
			*cp++ = HEX(ptr[i],   1);
			*cp++ = HEX(ptr[i],   0);
		    } else
			*cp++ = ' ', *cp++ = ' ';
		}
		*cp++ = ' ';
		*cp++ = ' ';
		for (i = 0; i < count; i++)
		    *cp++ = isprint(ptr[i]) ? ptr[i] : '.';
		*cp++ = '\n';
		*cp = 0;
		fputs(line, stdout);
		flag = 0;
	    } else if (!flag) {
		fputs("*\n", stdout);
		flag = 1;
	    }
	    address += count;
	    last = ptr;
	}
	buffer = (buffer == buffer1) ? buffer2 : buffer1;
    }
    cp = line;
    *cp++ = HEX(address, 5);
    *cp++ = HEX(address, 4);
    *cp++ = HEX(address, 3);
    *cp++ = HEX(address, 2);
    *cp++ = HEX(address, 1);
    *cp++ = HEX(address, 0);
    *cp++ = '\n';
    *cp = 0;
    fputs(line, stdout);

    if (nbytes < 0)
	perror("read");
}
