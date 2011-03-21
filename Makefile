CFLAGS = -Wall -Wextra -pedantic

BINS = ls2 makeheaders rot13 eolcheck \
			 shhcat trim crypt sentc pcp \
			 textwide textstrike bytes rcat2 \
			 scanfcat slowcat cat_isprint

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

%:%.c
	@echo CC $@
	@${CC} ${CFLAGS} -o $@ $^

# extra options
crypt:crypt.o
	@echo CC -lcrypt $@
	@${CC} ${CFLAGS} -lcrypt -o $@ $^

# extra deps
cf:         strings.o
pcp:        lib.o
crypt:      lib.o
textwide:   lib.o
textstrike: lib.o
bytes:      lib.o
