#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#define ARGV_WIDE   "textwide"
#define ARGV_STRIKE "textstrike"

#include "lib.h"

void process(FILE *f, void (*handle)(char))
{
	char buffa[256];

	while(fgets(buffa, 256, f)){
		char *c;
		for(c = buffa; *c; ++c)
			handle(*c);
	}

	if(ferror(f))
		perror("fgets()");
}

void strike(char c)
{
	if(c == '\n')
		putwchar('\n');
	else
		printf("%lc%lc", (int16_t)c, (int16_t)0x336);
}

void wide(char c)
{
	printf("%lc", '!' <= c && c <= '~' ? c + 0xfee0
		: c == ' ' ? 0x3000
		: c);
}

int main(int argc, char **argv)
{
	void (*func)(char);
	char *argv0 = *argv;
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
				    "Must be invoked as " ARGV_WIDE " or " ARGV_STRIKE "\n", argv0);
		return 1;
	}

	if(argc > 1)
		for(i = 1; i < argc; i++){
			FILE *f = fopen(argv[i], "r");
			if(!f){
				perrorf("%s: ", argv[i]);
				continue;
			}
			process(f, func);
			fclose(f);
		}
	else
		process(stdin, func);

	return 0;
}
