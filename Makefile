CC = gcc
CFLAGS = -Wall

gimginfo: gimginfo.o util.o sf_tre.o sf_typ.o

gimgxor: gimgxor.o

gimgunlock: gimgunlock.o

.PHONY: clean
clean:
	rm -f gimginfo gimgxor gimgunlock *.o
