#include <stdio.h>
#include <ctype.h>

int sentence(FILE *f)
{
	int c, capnext = 1;

	while((c = fgetc(f)) != EOF){
		if(!isalpha(c) && c != '\''){
			capnext = 1;
		}else if(capnext){
			capnext = 0;
			c = toupper(c);
		}
		putchar(c);
	}

	return ferror(f);
}

int main(int argc, const char **argv)
{
	int i, ret = 0;
	FILE *f;

	if(argc > 1)
		for(i = 1; i < argc; i++){
			f = fopen(argv[i], "r");
			if(f){
				ret += sentence(f);
				fclose(f);
			}else
				perror(argv[i]);
		}
	else
		ret += sentence(stdin);

	return ret;
}
