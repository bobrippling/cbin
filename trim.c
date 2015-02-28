#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>

int max = -1;
int max_arg = 0;

int gotwinch = 0;
int term_fd = -1;

int get_term_size()
{
#ifdef TIOCGSIZE
	struct ttysize ts;
# define COLS ts.ts_cols
# define ROWS ts.ts_lines
# define FLAG TIOCGSIZE
#else
	struct winsize ts;
# define COLS ts.ws_col
# define ROWS ts.ws_row
# define FLAG TIOCGWINSZ
#endif
	if(ioctl(term_fd, FLAG, &ts) == -1){
		perror("ioctl");
		return 1;
	}

	max = COLS;
	return 0;
}

void trim(FILE *f)
{
	int c, count = 0;
cont:
	while(errno = 0, (c = fgetc(f)) != EOF){
		if(gotwinch){
			gotwinch = 0;
			get_term_size(); /* ignore error */
		}

		if(c == '\n'){
			count = 0;
			putchar(c);
		}else if(++count <= max){
			if(c == '\t'){
				count++; /* FIXME */
				fputs("  ", stdout);
			}else
				putchar(c);
		}
	}

	if(errno == EINTR)
		goto cont;
}

void winch(int sig)
{
	(void)sig;
	gotwinch = 1;
	signal(SIGWINCH, winch);
}

int main(int argc, char **argv)
{
	int i;

	signal(SIGWINCH, winch);

	if(!isatty(STDERR_FILENO)){
		if(!isatty(STDOUT_FILENO)){
			if(!isatty(STDIN_FILENO)){
				fputs("not a tty\n", stderr);
				return 1;
			}else
				term_fd = STDIN_FILENO;
		}else
			term_fd = STDOUT_FILENO;
	}else
		term_fd = STDERR_FILENO;


	i = 1;
	if(argc > 1 && *argv[1] == '-'){
		if(!strcmp(argv[1], "--")){
			i++;
		}else{
			max_arg = 1;
			if(sscanf(argv[i] + 1, "%d", &max) != 1){
				fprintf(stderr, "Usage: %s [-N] [FILES...]\n", *argv);
				return 1;
			}
		}
	}

	if(!max_arg){
		if(get_term_size())
			return 1;
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
