CFLAGS = -Wall 
GIMGLIB_SOURCES = gimglib.c util.c sf_typ.c sf_mps.c sf_tre.c sf_rgn.c sf_lbl.c sf_net.c sf_nod.c sf_dem.c sf_mar.c sf_gmp.c
GIMGLIB_OBJS = $(GIMGLIB_SOURCES:.c=.o)
LIBS= -liconv

all: gimginfo gimgfixcmd gimgxor gimgunlock gimgch gimgextract cmdc

gimginfo: gimginfo.o $(GIMGLIB_OBJS)
	$(CC) -o $@ $^ ${LIBS}

gimgfixcmd: gimgfixcmd.o cmdlib.o $(GIMGLIB_OBJS)
	$(CC) -o $@ $^ ${LIBS}

gimgxor: gimgxor.o
	$(CC) -o $@ $^ ${LIBS}

gimgunlock: gimgunlock.o util_indep.o
	$(CC) -o $@ $^ ${LIBS}

gimgch: gimgch.o util_indep.o
	$(CC) -o $@ $^ ${LIBS}

gimgextract: gimgextract.o util_indep.o
	$(CC) -o $@ $^ ${LIBS}

cmdc: cmdc.o
	$(CC) -o $@ $^ ${LIBS}

.PHONY: clean
clean:
	rm -f gimginfo gimgfixcmd gimgxor gimgunlock gimgch gimgextract cmdc *.o
