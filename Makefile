CC = gcc
CFLAGS = -Wall

gimginfo: gimginfo.o util.o sf_tre.o sf_typ.o sf_mps.o

gimgxor: gimgxor.o

gimgunlock: gimgunlock.o

gimgdh: gimgdh.o

cmdc: cmdc.o

.PHONY: clean
clean:
	rm -f gimginfo gimgxor gimgunlock gimgdh cmdc *.o
