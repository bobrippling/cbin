CFLAGS = -Wall -Wextra -pedantic -std=c99

BINS = rot13 eolcheck shhcat trim \
			 crypt sentc pcp textwide \
			 textstrike textflip nc2 \
			 slowcat argv stdouterr

.PHONY: clean all mostlyclean

all: ${BINS}

clean: mostlyclean
	rm -f ${BINS}

mostlyclean:
	rm -f *.o

textwide: unicode.c
	${CC} ${CFLAGS} -o $@ $^
textstrike: unicode.c
	${CC} ${CFLAGS} -o $@ $^
textflip: unicode.c
	${CC} ${CFLAGS} -o $@ $^

%:%.c
	${CC} ${CFLAGS} -o $@ $^

# extra options
crypt:crypt.o
	${CC} ${CFLAGS} -lcrypt -o $@ $^

# extra deps
pcp:        lib.o
crypt:      lib.o
textwide:   lib.o
textstrike: lib.o
textflip:   lib.o
