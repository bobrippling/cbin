#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/select.h>

#include <unistd.h>

void check(int ret, const char *cmd)
{
	if(ret >= 0)
		return;

	fprintf(stderr, "%s(): %s\n", cmd, strerror(errno));
	exit(1);
}

void writes(int fd, const char *s)
{
	write(fd, s, strlen(s));
}

void read_and_print(int *fd, const char *colour)
{
	writes(1, colour);

	char buf[4096];
	ssize_t n = read(*fd, buf, sizeof buf);
	check(n, "read");
	if(n == 0){
		close(*fd);
		*fd = -1;
		return;
	}

	write(1, buf, n);

	writes(1, "\x1b[m");
}

int main(int argc, char *argv[])
{
	if(argc == 1){
		fprintf(stderr, "Usage: %s command...\n", *argv);
		return 1;
	}

	enum { read_i = 0, write_i = 1 };

	int child_err[2], child_out[2];
	check(pipe(child_err), "pipe");
	check(pipe(child_out), "pipe");

	long pid = fork();
	check(pid, "fork");

	if(pid == 0){
		close(child_out[read_i]);
		close(child_err[read_i]);

		check(dup2(child_out[write_i], 1), "dup2");
		check(dup2(child_err[write_i], 2), "dup2");

		close(child_out[write_i]);
		close(child_err[write_i]);

		execvp(argv[1], argv + 1);
		check(-1, "exec");
	}

	close(child_err[write_i]);
	close(child_out[write_i]);

	for(;;){
		fd_set fds;

		FD_ZERO(&fds);

		if(child_out[read_i] == -1
		&& child_err[read_i] == -1)
		{
			break;
		}

		int maxfd = 0;

		if(child_out[read_i] != -1){
			FD_SET(child_out[read_i], &fds);
			maxfd = child_out[read_i];
		}

		if(child_err[read_i] != -1){
			FD_SET(child_err[read_i], &fds);
			if(child_err[read_i] > maxfd)
				maxfd = child_err[read_i];
		}

		check(select(maxfd + 1, &fds, NULL, NULL, NULL), "select");

		if(FD_ISSET(child_out[read_i], &fds)){
			read_and_print(&child_out[read_i], "\x1b[1;34m");
		}

		if(FD_ISSET(child_err[read_i], &fds)){
			read_and_print(&child_err[read_i], "\x1b[1;35m");
		}
	}

	return 0;
}
