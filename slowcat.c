#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define MS_TO_NANO 100000

long int sleep_time;

int cat(FILE *f)
{
	struct timespec tspc;
	int c;

	while((c = fgetc(f)) != EOF){
		putchar(c);

		tspc.tv_sec  = 0;
		tspc.tv_nsec = sleep_time;
		nanosleep(&tspc, NULL);
	}

	return ferror(f);
}

int main(int argc, char **argv)
{
	int i, ret = 0;

	setbuf(stdout, NULL);

	sleep_time = 100; /* 1/10 sec */

	if(argc > 1)
		if(*argv[1] == '-' && sscanf(argv[1]+1, "%ld", &sleep_time) == 1){
			argc--;
			memmove(argv + 1, argv + 2, argc * sizeof(char *));
		}

	sleep_time *= MS_TO_NANO;

	if(argc == 1)
		return cat(stdin);

	for(i = 1; i < argc; i++){
		FILE *f = fopen(argv[i], "r");
		if(!f){
			fprintf(stderr, "%s: open: %s: %s\n",
					*argv, argv[i], strerror(errno));
		}else{
			ret |= cat(f);
			fclose(f);
		}
	}

	return ret;
}
