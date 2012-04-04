/*
 * $Id: xd.c,v 1.11 2012/04/04 17:58:42 urs Exp $
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#define BSIZE 4096

struct xdstate {
    unsigned char lbuf[16];
    unsigned char *last;
    int addr;
    int flag;
};

static void dump_file(const char *fname);
static void dump_init(struct xdstate *st, int addr);
static void dump_finish(char *dst, struct xdstate *st);
static void dump(char *dst, const void *src, int len, struct xdstate *st);
static void address(char *dst, int addr, char term);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    if (argc > 1)
	while (++argv, --argc)
	    dump_file(*argv);
    else
	dump_file("-");

    return 0;
}

static void dump_file(const char *fname)
{
    unsigned char buffer[BSIZE];
    char out[5 * BSIZE];
    int nbytes;
    FILE *fp;
    struct xdstate st;

    if (strcmp(fname, "-") == 0)
	fp = stdin;
    else if (!(fp = fopen(fname, "r"))) {
	perror(fname);
	return;
    }

    dump_init(&st, 0);

    do {
	nbytes = fread(buffer, 1, BSIZE, fp);
	dump(out, buffer, nbytes, &st);
	fputs(out, stdout);
    } while (nbytes == BSIZE);

    dump_finish(out, &st);
    fputs(out, stdout);

    if (ferror(fp))
	perror(fname);
    if (fp != stdin)
	fclose(fp);
}

#define HEX(n, i) ("0123456789abcdef"[((n) >> 4 * i) & 0xf])

static void dump_init(struct xdstate *st, int addr)
{
    st->flag = 0;
    st->last = NULL;
    st->addr = addr;
}

static void dump_finish(char *dst, struct xdstate *st)
{
    address(dst, st->addr, '\n');
}

static void dump(char *dst, const void *src, int len, struct xdstate *st)
{
    const unsigned char *buffer = src;
    const unsigned char *last, *ptr;
    char *cp = dst;
    int addr, count, ident, flag, i;

    last = st->last;
    flag = st->flag;
    addr = st->addr;

    for (ptr = buffer; ptr < buffer + len; ptr += 16) {
	count = len - (ptr - buffer);
	if (count >= 16) {
	    count = 16;
	    ident = last && memcmp(last, ptr, 16) == 0;
	} else
	    ident = 0;

	if (!ident) {
	    address(cp, addr, ' ');
	    cp += 9;
	    for (i = 0; i < 16; i++) {
		if (i % 2 == 0)
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

    if (last)
	memcpy(st->lbuf, last, 16);
    st->last = st->lbuf;
    st->flag = flag;
    st->addr = addr;
}

static void address(char *dst, int addr, char term)
{
    *dst++ = HEX(addr, 7);
    *dst++ = HEX(addr, 6);
    *dst++ = HEX(addr, 5);
    *dst++ = HEX(addr, 4);
    *dst++ = HEX(addr, 3);
    *dst++ = HEX(addr, 2);
    *dst++ = HEX(addr, 1);
    *dst++ = HEX(addr, 0);
    *dst++ = term;
    *dst = 0;
}
