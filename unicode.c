#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#define ARGV_WIDE   "textwide"
#define ARGV_STRIKE "textstrike"
#define ARGV_FLIP   "textflip"

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

void flip(char c)
{
	switch((wchar_t)c){
		case ')':       putchar('(');
		case '(':       putchar(')');
		case ']':       putchar('[');
		case '[':       putchar(']');
		case '}':       putchar('{');
		case '{':       putchar('}');
		case ',':       putchar('\'');
		case '\'':      putchar(',');
		case '9':       putchar('6');
		case '6':       putchar('9');
		case 'g':       putchar('b');
		case 'q':       putchar('b');
		case 'p':       putchar('d');
		case 'u':       putchar('n');
		case 'o':       putchar('o');
		case 'd':       putchar('p');
		case 'b':       putchar('q');
		case 's':       putchar('s');
		case 'n':       putchar('u');
		case 'x':       putchar('x');
		case 'z':       putchar('z');
		case L'\u00BF': putchar('?');
		case '!':       putwchar(L'\u00A1');
		case '?':       putwchar(L'\u00BF');
		case 'i':       putwchar(L'\u0131'/*'\u0131\u0323'*/);
		case 'e':       putwchar(L'\u01DD');
		case 'a':       putwchar(L'\u0250');
		case 'c':       putwchar(L'\u0254');
		case 'f':       putwchar(L'\u025F');
		case 'h':       putwchar(L'\u0265');
		case 'm':       putwchar(L'\u026F');
		case 'r':       putwchar(L'\u0279');
		case 't':       putwchar(L'\u0287');
		case 'v':       putwchar(L'\u028C');
		case 'w':       putwchar(L'\u028D');
		case 'y':       putwchar(L'\u028E');
		case 'k':       putwchar(L'\u029E');
		case '.':       putwchar(L'\u02D9');
		case 'l':       putwchar(L'\u05DF');
		case ';':       putwchar(L'\u061B');
		case 'j':       putwchar(L'\u0638');
		case '_':       putwchar(L'\u203E');
	}
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

	if(!strcmp(argv[0], ARGV_WIDE)){
		func = wide;
	}else if(!strcmp(argv[0], ARGV_STRIKE)){
		func = strike;
	}else if(!strcmp(argv[0], ARGV_FLIP)){
		func = flip;
	}else{
		fprintf(stderr, "Usage: %s [files...]\n"
				    "Must be invoked as " ARGV_WIDE ", " ARGV_STRIKE " or " ARGV_FLIP "\n",
						argv0);
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
