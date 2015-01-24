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
	@echo CC $@
	@${CC} ${CFLAGS} -o $@ $^
textstrike: unicode.c
	@echo CC $@
	@${CC} ${CFLAGS} -o $@ $^
textflip: unicode.c
	@echo CC $@
	@${CC} ${CFLAGS} -o $@ $^

%:%.c
	@echo CC $@
	@${CC} ${CFLAGS} -o $@ $^

# extra options
crypt:crypt.o
	@echo CC -lcrypt $@
	@${CC} ${CFLAGS} -lcrypt -o $@ $^

# extra deps
pcp:        lib.o
crypt:      lib.o
textwide:   lib.o
textstrike: lib.o
textflip:   lib.o
