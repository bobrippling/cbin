#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

static void checkfile(char *);
static void usage(char *);
static int yesno(void);
static int getsinglechar(void);


static int prompt = 1, append = 0;


static int getsinglechar(void)
{
	int c = getchar();

	if(c != '\n'){
		/* character entered - clear the '\n' after it and whatever else */
		int tmp;
		do
			tmp = getchar();
		while(tmp != EOF && tmp != '\n');
	}

	return c;
}


static int yesno(void)
{
	char c = getsinglechar();

	if(c == '\n' || tolower(c) == 'y')
		return 1;
	return 0;
}


static void checkfile(char *fname)
{
	struct stat st;

	if(lstat(fname, &st) != 0)
		fputs(strerror(errno), stdout);
	else if(S_ISDIR(st.st_mode))
		fputs("dir", stdout);
	else{
		FILE *f = fopen(fname, "r");

		if(!f || fseek(f, 0, SEEK_END)){
			fputs(strerror(errno), stdout);
		}else{
			/* now at eof */
			long pos = ftell(f);

			if(pos > 0){
				if(fseek(f, pos - 1, SEEK_SET))
					fputs(strerror(errno), stdout);
				else{
					if(fgetc(f) == '\n')
						fputs("eol", stdout);
					else{
						if(append){
							int add;
							if(prompt){
								printf("	\"%s\" - append newline? (Y/n) ", fname);
								add = yesno();
							}else
								add = 1;

							if(add){
								fclose(f);
								f = fopen(fname, "a");
								if(f){
									if(fputc('\n', f) == EOF)
										printf("append: %s", strerror(errno));
									else
										fputs("eol (added)", stdout);
									/* file closed below */
								}else
									fprintf(stderr, "open: %s", strerror(errno));
							}else
								fputs("noeol", stdout);
						}else
							fputs("noeol", stdout);
					}
				}
			}else
				fputs("zero length", stdout);
		}

		if(f)
			fclose(f);
	}

	printf(" - %s\n", fname);
}

static void usage(char *progname)
{
	fprintf(stderr, "Usage: %s [options] FILE(S)\n", progname);
	fputs("options can be:\n", stderr);
	fputs("	-n: Ask to add a new line\n", stderr);
	fputs("	-f: Don't prompt - just add a newline\n", stderr);
	fputs("		(Implies -n)\n", stderr);
	exit(1);
}


int main(int argc, char **argv)
{
	if(argc > 1){
		int i = 1, processargs = 1;

		/*
		 * order matters - ./$0 -- -n will segfault,
		 * since argv[i][0] is accessed after inc'ing i for --
		 * , if processargs is checked second
		 */
		while(processargs && argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'f':
					if(!prompt)
						usage(argv[0]);
					prompt = 0;
					append = 0; /* minor hack for fall through */

				case 'n':
					if(append)
						usage(argv[0]);
					append = 1;
					break;

				case '-':
					if(strcmp(argv[i], "--"))
						/* argv[i] isn't "--", so decrease i and exit, so that this arg is checked */
						i--;
					/* either way, process this 'file' */

				default:
					processargs = 0;
					break;
			}
			i++;
		}

		if(i >= argc)
			fputs("No input files\n", stderr);
		else
			while(i < argc)
				checkfile(argv[i++]);

	}else{
		usage(argv[0]);
	}

	return 0;
}
