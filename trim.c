#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
	/* TODO: argv: -40, etc */
	int c, count = 0, fno;

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

	 /* check if stdin is connected to a terminal-device */
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

	while((c = getchar()) != EOF)
		if(c == '\n'){
			count = 0;
			putchar(c);
		}else if(++count <= COLS){
			if(c == '\t'){
				count++;
				fputs("  ", stdout);
			}else
				putchar(c);
		}

	return 0;
}
