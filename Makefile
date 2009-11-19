CC = gcc
CFLAGS = -Wall

gimginfo: gimginfo.o gimglib.o util.o sf_tre.o sf_typ.o sf_mps.o

gimgxor: gimgxor.o

gimgunlock: gimgunlock.o

gimgdh: gimgdh.o

gimgextract: gimgextract.o

cmdc: cmdc.c
	gcc -o cmdc -Wall -lm cmdc.c

.PHONY: clean
clean:
	rm -f gimginfo gimgxor gimgunlock gimgdh gimgextract cmdc *.o
