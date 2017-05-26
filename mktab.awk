#!/usr/bin/awk -f

BEGIN {
    print "/* Automatically generated.  Don't edit. */\n"
    print "static const char hextab[][2] = {"
    for (n = 0; n < 256; n++)
	printf " \"%.2x\",%s", n, n % 16 < 15 ? "" : "\n"
    print "};"
}
