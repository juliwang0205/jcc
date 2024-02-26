CFLAGS=-std=c11 -g -fno-common

jcc: main.o
	$(CC) -o jcc main.o $(LDCFLAGS)

test: jcc
	./test.sh

clean:
	rm -f jcc *.o *~ tmp*

.PHONY: test clean