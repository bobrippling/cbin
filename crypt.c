#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"

const char *progname;
int decrypt = 0;

char randsaltchar(void);
int cryptfile(FILE *f);
ssize_t getline2(char **lineptr, size_t *n, FILE *stream);

void usage(void);

ssize_t getline2(char **lineptr, size_t *n, FILE *stream)
{
	ssize_t t = getline(lineptr, n, stream);
	if(lineptr && *lineptr){
		char *p = strchr(*lineptr, '\n');
		if(p)
			*p = '\0';
	}
	return t;
}

char randsaltchar(void)
{
	/*[a–zA–Z0–9./]*/
	int r = rand() % (26 * 2 + 10 + 2);

	if(r < 26)
		return r + 'a';
	else if(r < 26 * 2)
		return r + 'A';
	else if(r < 26 * 2 + 10)
		return r + '0';
	else
		return r % 2 ? '.' : '/';
}

int cryptfile(FILE *f)
{
	size_t keysize = 0;
	char *key = NULL;
	char salt[2];

	if(decrypt){
		ssize_t passsize = 0;
		char *pass = NULL;

		do{
			salt[0] = randsaltchar();
			salt[1] = randsaltchar();

			if(getline2(&pass, &passsize, f) == -1)
				goto bail;
			if(getline2(&key, &keysize, f) == -1)
				goto bail;

			/* if crypt(pass, pass_cryptedpass) == pass_cryptedpass */
			if(!strcmp(crypt(pass, key), key))
				puts("Correct");
			else
				puts("Incorrect");
		}while(1);
bail:
		free(pass);
	}else
		while(getline2(&key, &keysize, f) != -1){
			salt[0] = randsaltchar();
			salt[1] = randsaltchar();

			printf("%s\n", crypt(key, salt));
		}

	free(key);
	return !!ferror(f);
}

void usage(void)
{
	fprintf(stderr,
			"Usage: %s [OPTIONS] [FILES...]\n",
			"\n"
			"Options:\n"
			" -c: Verify entered password with entered crypt-string\n"
			"     Takes two lines - password, followed by crypt-string\n"
			, progname);
	exit(1);
}

int main(int argc, const char **argv)
{
	int i, ret = 0;

	progname = *argv++;
	argc--;
	srand(time(NULL));

	if(argc > 0)
		if(!strcmp(*argv, "-c")){
			decrypt = 1;
			argc--;
			argv++;
		}

	if(*argv)
		while(*argv){
			FILE *f = fopen(*argv, "r");
			if(f){
				ret += cryptfile(f);
				fclose(f);
			}else{
				perrorf("open: %s", argv[i]);
				ret++;
			}
			argv++;
		}
	else
		ret += cryptfile(stdin);

	return ret;
}
