#include <stdio.h>
#include <stdarg.h>

#include "lib.h"

void perrorf(const char *fmt, ...)
{
	va_list l;
	extern const char *__progname;

	fprintf(stderr, "%s: ", __progname);
	va_start(l, fmt);
	vfprintf(stderr, fmt, l);
	va_end(l);
	fputs(": ", stderr);
	perror(NULL);
}
