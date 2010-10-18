#include <stdio.h>
#include <stdlib.h>

void usage(const char *n)
{
	fprintf(stderr, "Usage: %s bytes-to-extract\n", n);
	exit(1);
}

int bytes(FILE *f, int n)
{
	int c;

	while(n --> 0 && (c = fgetc(f)) != EOF)
		putchar(c);

	return ferror(f);
}

int main(int argc, char **argv)
{
#define USAGE() usage(*argv)
	int i, ret = 0, nbytes;

	if(argc < 2)
		USAGE();

	if(sscanf(argv[1], "%d", &nbytes) != 1)
		USAGE();
	if(nbytes <= 0)
		USAGE();

	if(2 == argc)
		ret += bytes(stdin, nbytes);
	else
		for(i = 2; i < argc; i++){
			FILE *f = fopen(argv[i], "r");
			if(f){
				ret += bytes(f, nbytes);
				fclose(f);
			}else{
				perrorf("%s: %s: ", *argv, argv[i]);
				ret++;
			}
		}

	return 0;
}
