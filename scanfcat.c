#include <stdio.h>
#include <ctype.h>

int sfc(FILE *f)
{
	char buffer[1024], *s;

	while(fgets(buffer, sizeof buffer, f))
		for(s = buffer; *s; s++)
#define c (*s)
			if(isprint(c) || c == '\n')
				putchar(c);
			else
				printf("\\%d", (int)c);
	return !!ferror(f);
}

int main(int argc, char **argv)
{
	int i, ret = 0;

	if(argc == 1)
		return sfc(stdin);

	for(i = 1; i < argc; i++){
		FILE *f = fopen(argv[i], "r");
		if(!f){
			perror(argv[i]);
			ret++;
		}else{
			ret += sfc(f);
			fclose(f);
		}
	}

	return ret;
}
