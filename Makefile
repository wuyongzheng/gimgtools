CC = gcc
CFLAGS = -Wall

gimginfo: gimginfo.o util.o

gimgxor: gimgxor.o

dumptre: dumptre.o

.PHONY: clean
clean:
	rm -f gimginfo gimgxor dumptre *.o
