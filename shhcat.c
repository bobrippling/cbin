#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CAT(in) \
		while(read(in, &buf, sizeof buf) > 0) \
			write(STDOUT_FILENO, &buf, sizeof buf)

int main(int argc, char **argv)
{
	struct termios attr, origattr;
	int i;
	char buf;

	if(!isatty(STDIN_FILENO)){
		fputs("stdin not a tty\n", stderr);
		return 1;
	}

	if(tcgetattr(STDIN_FILENO, &attr)){
		perror("tcgetattr()");
		return 1;
	}

	memcpy(&origattr, &attr, sizeof attr);

	attr.c_lflag &= ~ECHO;

	if(tcsetattr(STDIN_FILENO, TCSANOW, &attr)){
		perror("tcsetattr()");
		return 1;
	}

	/* good to go */
	if(argc == 1)
		CAT(STDIN_FILENO);
	else
		for(i = 1; i < argc; i++){
			int fd = open(argv[i], O_RDONLY);
			if(fd == -1)
				perror(argv[i]);
			else{
				CAT(fd);
				close(fd);
			}
		}

	if(tcsetattr(STDIN_FILENO, TCSANOW, &origattr)){
		perror("(restore) tcsetattr()");
		return 1;
	}

	return 0;
}
