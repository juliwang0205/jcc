CFLAGS=-std=c11 -g -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

jcc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDCFLAGS)

$(OBJS): jcc.h

test: jcc
	./test.sh

clean:
	rm -f jcc *.o *~ tmp*

.PHONY: test clean