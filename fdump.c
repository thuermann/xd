/*  fdump.c
 *  (c) 1989 by Urs Thuermann
 *  last modified: 13.11.1990
 */

#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE  !FALSE

void fprhex(FILE *fp, int digits, long value);

int main(int argc, char *argv[])
{
	FILE *fpin, *fpout;
	char asc[17], *cp;
	long offset;
	int i, c;
	int error, eof;

	error = 0;
	if (argc == 4)
		if (strcmp(argv[2], "-o") == 0 || strcmp(argv[2], "-O") == 0)
		{
			if (!(fpout = fopen(argv[3], "w")))
				error = 1;
		}
		else
			error = 2;
	else if (argc == 2)
		fpout = stdout;
	else
		error = 2;
	if (!(fpin = fopen(argv[1], "rb")))
		error = 1;
	if (error == 2)
	{
		fputs("Usage: fdump file [-o file]\n", stderr);
		return(-1);
	}
	if (error == 1)
	{
		fputs("File open error.\n", stderr);
		return(-1);
	}

	eof = FALSE;
	offset = 0;
	do
	{
		fprhex(fpout, 6, offset);
		fputc(' ', fpout);
		for (cp = asc, i = 0; i < 16; i++)
		{
			if (!(i & 1))
				fputc(' ', fpout);
			c = fgetc(fpin);
			if (c == EOF)
			{
				fputc(' ', fpout);
				fputc(' ', fpout);
				eof = TRUE;
			}
			else
			{
				fprhex(fpout, 2, c);
				*cp++ = c >= ' ' ? c : '.';
			}
		}
		*cp = 0;
		fputc(' ', fpout);
		fputs(asc, fpout);
		fputc('\n', fpout);
		offset += 16;
	} while (!eof);

	fclose(fpin);
	return(0);
}

void fprhex(FILE *fp, int digits, long value)
{
	static char hex[] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	static char str[] = "________";

	str[7] = hex[value & 0xF];
	str[6] = hex[(value >>= 4) & 0xF];
	if (digits > 2)
	{
		str[5] = hex[(value >>= 4) & 0xF];
		str[4] = hex[(value >>= 4) & 0xF];
		if (digits > 4)
		{
			str[3] = hex[(value >>= 4) & 0xF];
			str[2] = hex[(value >>= 4) & 0xF];
			str[1] = hex[(value >>= 4) & 0xF];
			str[0] = hex[(value >>= 4) & 0xF];
		}
	}
	fputs(str + 8 - digits, fp);
}
