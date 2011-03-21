#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <alloca.h>
#include <errno.h>
#include <limits.h>

#include "lib.h"

#define BSIZ 4096
#define PROGRESS_COUNT 30

int i_am_cp;
char *progname;

/* ----- erroring ----- */

void eprint(char *s)
{
	fprintf(stderr, "%s: %s\n", progname, s);
}

void eprintf(char *fmt, ...)
{
	va_list l;

	fprintf(stderr, "%s: ", progname);
	va_start(l, fmt);
	vfprintf(stderr, fmt, l);
	va_end(l);
	fputc('\n', stderr);
}

/* ----- copying ----- */

void progress(const char *name, size_t now, size_t total)
{
	printf("`%s': %3.2f%% (%ld/%ld)\r", name, 100.0 * now / total, now, total);
}

#define progressdone(name, siz) \
	do{ \
		progress(name, siz, siz); \
		putchar('\n'); \
	}while(0)

size_t filelen(char *file)
{
	struct stat st;

	if(stat(file, &st)){
		/*
		 * very slim chance that this will fail,
		 * since we've just fopen()'d file, so we
		 * should still have read access, unless it's
		 * been deleted/permission change'd during
		 */
		perrorf("filesize: `%s'", file);
		exit(1);
	}
	return st.st_size;
}

int filecopy(FILE *in, FILE *out, char *outname, size_t insiz)
{
	char buffer[BSIZ];
	size_t nread, total = 0;
	int ret = 0, lastprogress = 0;

	while((nread = fread(buffer, sizeof buffer[0], BSIZ, in)) > 0){
		if(fwrite(buffer, sizeof buffer[0], nread, out) <= 0){
			perrorf("fwrite()");
			ret = 1;
			goto bail;
		}
		if(++lastprogress > PROGRESS_COUNT){
			progress(outname, total += nread, insiz);
			lastprogress = 0;
		}
	}

	progressdone(outname, insiz);

	if(ferror(in)){
		perrorf("fread()");
		ret = 1;
	}
bail:
	return ret;
}

int samefile(char *a, char *b)
{
	struct stat st;
	unsigned int inode;

	if(stat(a, &st)){
		if(errno == ENOENT)
			return 0;
		perrorf("stat: `%s'", a);
		exit(1);
	}
	inode = st.st_ino;

	if(stat(b, &st)){
		if(errno == ENOENT)
			return 0;
		perrorf("stat: `%s'", b);
		exit(1);
	}

	return inode == st.st_ino;
}

int copy(char *dest, char *src)
{
	FILE *in, *out;
	int ret;
	char *actualdest = dest;

	if(samefile(dest, src)){
		eprintf("`%s' and `%s' are the same file", dest, src);
		return 1;
	}

	if(!(in = fopen(src, "r"))){
		perrorf("open (for read): `%s'", src);
		return 1;
	}

	/* TODO: make dir if it doesn't exist */
	if(!(out = fopen(dest, "w"))){
		if(errno == EISDIR){
			char *srcbase = strrchr(src, '/');

			if(!srcbase)
				srcbase = src;

			actualdest = alloca(strlen(dest) + strlen(srcbase) + 2);
			sprintf(actualdest, "%s/%s", dest, srcbase);

			if(samefile(actualdest, src)){
				eprintf("`%s' and `%s' are the same file", actualdest, src);
				fclose(in);
				return 1;
			}

			out = fopen(actualdest, "w");
			if(!out){
				perrorf("open (for write): `%s'", actualdest);
				fclose(in);
				return 1;
			}
		}else{
			perrorf("open (for write): `%s'", dest);
			fclose(in);
			return 1;
		}
	}

	ret = filecopy(in, out, actualdest, filelen(src));

	fclose(in);
	fclose(out);

	if(!i_am_cp && remove(src))
		perrorf("non-fatal: remove: `%s'", src);

	return ret;
}

int dircopy(char *base, char *src[], int nsrc)
{
	char *dest = NULL;
	int baselen = strlen(base) + 1; /* +1 for '/' */
	int i, destlen = 0, ret = 0;
#define BAIL() do{ ret = 1; goto bail; } while(0)

	for(i = 0; i < nsrc; i++){
		int newlen = baselen + strlen(src[i]) + 1;

		if(destlen < newlen){
			char *tmp = realloc(dest, newlen);
			if(!tmp){
				perrorf("realloc()");
				BAIL();
			}
			dest = tmp;
		}

		if(*src[i] != '/')
			sprintf(dest, "%s/%s", base, src[i]);
		else
			strcpy(dest, src[i]);

		if(copy(dest, src[i]))
			BAIL();
	}

bail:
	free(dest);

	return ret;
}

void usage(void)
{
	fprintf(stderr, "Usage: %s SOURCE DEST\n", progname);
	fputs(          "           SOURCES... DIR\n"
	                "           DIR\n"
	                "with the final case reading source filenames\n"
	                "on the standard input\n",  stderr);
	exit(1);
}


int main(int argc, char **argv)
{
#define nsrc (argc - 2)
	char *dest;

	progname = strrchr(*argv, '/');
	if(progname)
		progname++;
	else
		progname = *argv;

	setvbuf(stdout, NULL, _IONBF, 0);

	if(!strcmp(progname, "pcp"))
		i_am_cp = 1;
	else if(!strcmp(progname, "pmv"))
		i_am_cp = 0;
	else{
		eprint("must be invoked as ``pcp'' or ``pmv''");
		return 1;
	}

	if(argc == 1)
		usage();

	if(setvbuf(stdout, NULL, _IONBF, 0)){
		perrorf("unbuffer stdout");
		return 1;
	}
	dest = argv[argc-1];

	if(nsrc == 1)
		return copy(dest, argv[1]);
	else{
		/* either way, dest must be a dir */
		struct stat st;

		if(stat(dest, &st)){
			perrorf("stat: `%s'", dest);
			return 1;
		}else if(!S_ISDIR(st.st_mode)){
			eprintf("`%s': not a dir", dest);
			return 1;
		}

		if(nsrc)
			return dircopy(dest, argv + 1, nsrc);
		else{
			char fname[PATH_MAX];

			while(fgets(fname, PATH_MAX, stdin)){
				char *nl = strchr(fname, '\n');
				if(nl)
					*nl = '\0';
				else
					eprintf("warning: full line not read for `%s'\n", fname);

				if(copy(dest, fname))
					return 1;
			}
			return 0;
		}
	}
}
