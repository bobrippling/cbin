#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv)
{
#define STATUS(s) if(verbose) fputs(":: " s "\n", stderr)
	int sock, ret = 0, verbose = 0, serv = 0, i;
	struct sockaddr_in addr;
	struct hostent *hent;
	struct linger linger;
	char buffer[256], *host = NULL, *port = NULL;

	for(i = 1; i < argc; i++)
		if(!strcmp(argv[i], "-v"))
			verbose = 1;
		else if(!strcmp(argv[i], "-l"))
			serv = 1;
		else if(!host)
			host = argv[i];
		else if(!port)
			port = argv[i];
		else{
usage:
			fprintf(stderr, "Usage: %s [-v] [-l] host port\n", *argv);
			return 1;
		}

	if(serv){
		if(host && !port){
			port = host;
			host = NULL;
		}else
			goto usage;
	}

	if((!host && !serv) || !port)
		goto usage;

	memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket()");
		return 1;
	}

	/* fancy socket bizniz... */
	linger.l_onoff = 1;
	linger.l_linger = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger, sizeof linger) < 0){
		perror("setsockopt(SO_LINGER)");
		ret = 1;
		goto bail;
	}

	/* listen immediately afterwards */
	i = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof i) < 0){
		perror("setsockopt(SO_REUSEADDR)");
		ret = 1;
		goto bail;
	}

	if(serv){
		int client;
		struct sockaddr_in dummy;
		socklen_t dummylen = sizeof dummy;

		if(bind(sock, (struct sockaddr *)&addr, sizeof addr) == -1){
			perror("bind()");
			ret = 1;
			goto bail;
		}

		if(listen(sock, 0) == -1){
			perror("serv()");
			ret = 1;
			goto bail;
		}

		client = accept(sock, (struct sockaddr *)&dummy, &dummylen);
		if(client == -1){
			perror("accept()");
			ret = 1;
			goto bail;
		}
		close(sock);
		sock = client;
	}else{
		hent = gethostbyname(host);

		if(!hent){
			perror("gethostbyname()");
			return 1;
		}
		atexit(endhostent);
		memcpy(&addr.sin_addr, hent->h_addr_list[0], sizeof addr.sin_addr);

		if(connect(sock, (struct sockaddr *)&addr, sizeof addr) == -1){
			perror("connect()");
			ret = 1;
			goto bail;
		}
	}

	STATUS("connected");

	for(;;){
		int nbits, nread;
		fd_set rfds;

		FD_ZERO(&rfds);
		FD_SET(STDIN_FILENO, &rfds);
		FD_SET(sock, &rfds);

		if((nbits = select(sock+1, &rfds, NULL, NULL, NULL /* timeout */)) == -1){
			perror("select()");
			ret = 1;
			goto bail;
		}

		if(FD_ISSET(sock, &rfds)){
			switch((nread = read(sock, buffer, sizeof buffer))){
				case -1:
					perror("read()");
					ret = 1;
					goto bail;
				case 0:
					STATUS("remote end closed connection");
					goto bail;
			}
			fwrite(buffer, nread, sizeof *buffer, stdout);
		}
		if(FD_ISSET(STDIN_FILENO, &rfds)){
			if(fgets(buffer, sizeof buffer, stdin) == NULL)
				goto bail;
			if(write(sock, buffer, strlen(buffer)) == -1){
				perror("write()");
				ret = 1;
				goto bail;
			}
		}
	}

bail:
	close(sock);
	return ret;
}
