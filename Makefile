CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
GIMGLIB_SOURCES = gimglib.c util.c sf_typ.c sf_mps.c sf_tre.c sf_rgn.c sf_lbl.c sf_net.c sf_nod.c sf_gmp.c
GIMGLIB_OBJS = $(GIMGLIB_SOURCES:.c=.o)

all: gimginfo gimgfixcmd gimgxor gimgunlock gimgch gimgextract cmdc

gimginfo: gimginfo.o $(GIMGLIB_OBJS)

gimgfixcmd: gimgfixcmd.o cmdlib.o $(GIMGLIB_OBJS)
	$(CC) -o $@ $^ -lm

gimgxor: gimgxor.o

gimgunlock: gimgunlock.o util_indep.o

gimgch: gimgch.o util_indep.o

gimgextract: gimgextract.o util_indep.o

cmdc: cmdc.o
	$(CC) -o $@ $< -lm

.PHONY: clean
clean:
	rm -f gimginfo gimgfixcmd gimgxor gimgunlock gimgch gimgextract cmdc *.o
