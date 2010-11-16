#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int max = -1;

void trim(FILE *f)
{
	int c, count;
	while((c = fgetc(f)) != EOF)
		if(c == '\n'){
			count = 0;
			putchar(c);
		}else if(++count <= max){
			if(c == '\t'){
				count++;
				fputs("  ", stdout);
			}else
				putchar(c);
		}
}

int main(int argc, char **argv)
{
	int i;

#if TIOCGSIZE
	struct ttysize ts;
# define COLS ts.ts_cols
# define ROWS ts.ts_lines
# define FLAG TIOCGSIZE

#elif defined(TIOCGWINSZ)
	struct winsize ts;
# define COLS ts.ws_col
# define ROWS ts.ws_row
# define FLAG TIOCGWINSZ

#endif


	for(i = 1; i < argc; i++)
		if(*argv[i] == '-'){
			if(sscanf(argv[i] + 1, "%d", &max) != 1){
				fprintf(stderr, "Usage: %s [-N] [FILES...]\n", *argv);
				return 1;
			}
		}else
			break;

	if(max == -1){
		/* check if stdin is connected to a terminal-device */
		int fno;
		if(!isatty(STDERR_FILENO)){
			if(!isatty(STDOUT_FILENO)){
				if(!isatty(STDIN_FILENO)){
					fputs("not a tty\n", stderr);
					return 1;
				}else
					fno = STDIN_FILENO;
			}else
				fno = STDOUT_FILENO;
		}else
			fno = STDERR_FILENO;

		if(ioctl(fno, FLAG, &ts) == -1){
			perror("ioctl");
			return 1;
		}

		max = COLS;
	}

	if(i < argc)
		while(i < argc){
			FILE *f = fopen(argv[i], "r");
			if(!f)
				perror(argv[i]);
			else{
				trim(f);
				fclose(f);
			}
			i++;
		}
	else
		trim(stdin);

	return 0;
}
