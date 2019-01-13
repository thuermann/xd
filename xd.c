/*
 * $Id: xd.c,v 1.16 2019/01/13 00:54:26 urs Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [-g n] files...\n", name);
}

#define BSIZE 4096

struct xdstate {
    unsigned char lbuf[16];
    unsigned char *last;
    unsigned long long addr;
    int flag;
    int group;
};

static int  dump_file(const char *fname, int group);
static void dump_init(struct xdstate *st, unsigned long long addr, int group);
static void dump_finish(char *dst, struct xdstate *st);
static void dump(char *dst, const void *src, int len, struct xdstate *st);
static int  address(char *dst, unsigned long long addr, char term);

int main(int argc, char **argv)
{
    int errflg = 0, group = 2;
    int opt;

    setlocale(LC_ALL, "");

    while ((opt = getopt(argc, argv, "g:")) != -1) {
	switch (opt) {
	case 'g':
	    group = atoi(optarg);
	    break;
	default:
	    errflg = 1;
	    break;
	}
    }
    if (errflg) {
	usage(argv[0]);
	exit(1);
    }

    if (argc - optind > 0) {
	while (optind < argc)
	    if (dump_file(argv[optind++], group))
		errflg = 1;
    } else if (dump_file("-", group))
	errflg = 1;

    return errflg;
}

static int dump_file(const char *fname, int group)
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
	return 1;
    }

    dump_init(&st, 0, group);

    do {
	nbytes = fread(buffer, 1, BSIZE, fp);
	dump(out, buffer, nbytes, &st);
	fputs(out, stdout);
    } while (nbytes == BSIZE);

    dump_finish(out, &st);
    fputs(out, stdout);

    if (ferror(fp)) {
	perror(fname);
	return 1;
    }
    if (fp != stdin)
	fclose(fp);

    return 0;
}

#include "hextab.c"

static void dump_init(struct xdstate *st, unsigned long long addr, int group)
{
    st->flag  = 0;
    st->last  = NULL;
    st->addr  = addr;
    st->group = group;
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
    int count, gcount, ident, flag, i;
    unsigned long long addr;

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
	    cp += address(cp, addr, ' ');
	    gcount = 1;
	    for (i = 0; i < 16; i++) {
		if (--gcount == 0) {
		    *cp++ = ' ';
		    gcount = st->group;
		}
		if (i < count)
		    memcpy(cp, hextab[ptr[i]], 2), cp += 2;
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

static int address(char *dst, unsigned long long addr, char term)
{
    char *cp = dst;
    int n;

    for (n = 6; --n >= 0; )
	memcpy(cp, hextab[(addr >> (n * 8)) & 0xff], 2), cp += 2;
    *cp++ = term;
    *cp = 0;

    return cp - dst;
}
