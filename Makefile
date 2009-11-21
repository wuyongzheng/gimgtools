CC = gcc
CFLAGS = -Wall

gimginfo: gimginfo.o gimglib.o util.o sf_typ.o sf_mps.o sf_tre.o sf_rgn.o sf_lbl.o sf_net.o sf_nod.o sf_gmp.o

gimgxor: gimgxor.o

gimgunlock: gimgunlock.o

gimgch: gimgch.o

gimgextract: gimgextract.o

cmdc: cmdc.c
	gcc -o cmdc -Wall -lm cmdc.c

.PHONY: clean
clean:
	rm -f gimginfo gimgxor gimgunlock gimgch gimgextract cmdc *.o
