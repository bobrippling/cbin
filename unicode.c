#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <stdarg.h>

#define ARGV_WIDE   "textwide"
#define ARGV_STRIKE "textstrike"

/*
 * TODO: generalise the fgets() business
 */

void perrorf(const char *fmt, ...)
{
	va_list l;

	va_start(l, fmt);
	vfprintf(stderr, fmt, l);
	va_end(l);

	perror(NULL);
}

void strike(FILE *f)
{
	char buffa[256];

	while(fgets(buffa, 256, f)){
		char *c;
		for(c = buffa; *c; ++c)
			if(*c == '\n')
				putwchar('\n');
			else
				printf("%lc%lc", (int16_t)*c, (int16_t)0x336);
	}

	if(ferror(f))
		perror("fgets()");
}

void wide(FILE *f)
{
	char buffa[256];

	while(fgets(buffa, 256, f)){
		char *c;
		for(c = buffa; *c; ++c)
			printf("%lc", '!' <= *c && *c <= '~' ? *c + 0xfee0
				: *c == ' ' ? 0x3000
				: *c);
	}
}

int main(int argc, char **argv)
{
	void (*func)(FILE *);
	char *pos;
	int i;

	if(setlocale(LC_CTYPE, "") == NULL){
		fprintf(stderr, "Locale not specified. Fix this.\n");
		return 1;
	}

	pos = strrchr(argv[0], '/');
	if(pos)
		argv[0] = pos+1;

	if(!strcmp(argv[0], ARGV_WIDE))
		func = wide;
	else if(!strcmp(argv[0], ARGV_STRIKE))
		func = strike;
	else{
		fprintf(stderr, "Usage: %s [files...]\n"
				    "Must be invoked as " ARGV_WIDE " or " ARGV_STRIKE "\n", *argv);
		return 1;
	}

	if(argc > 1)
		for(i = 1; i < argc; i++){
			FILE *f = fopen(argv[i], "r");
			if(!f){
				perrorf("%s: %s: ", *argv, argv[i]);
				continue;
			}
			func(f);
			fclose(f);
		}
	else
		func(stdin);

	return 0;
}
