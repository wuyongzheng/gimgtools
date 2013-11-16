# For MacPorts iconv PREFIX=/opt/local 
# Set on command if necessary 
# make PREFIX=/opt/local

CC = gcc
CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
LDLIBS = -lm

ifdef PREFIX 
  CFLAGS += -I$(PREFIX)/include
  LDLIBS += -L$(PREFIX)/lib
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LDLIBS += -liconv
endif

GIMGLIB_SOURCES = gimglib.c util.c sf_typ.c sf_mps.c sf_tre.c sf_rgn.c sf_lbl.c sf_net.c sf_nod.c sf_dem.c sf_mar.c sf_gmp.c
GIMGLIB_OBJS = $(GIMGLIB_SOURCES:.c=.o)

all: gimginfo gimgfixcmd gimgxor gimgunlock gimgch gimgextract cmdc

gimginfo: gimginfo.o $(GIMGLIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

gimgfixcmd: gimgfixcmd.o cmdlib.o $(GIMGLIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

gimgxor: gimgxor.o

gimgunlock: gimgunlock.o util_indep.o

gimgch: gimgch.o util_indep.o

gimgextract: gimgextract.o util_indep.o

cmdc: cmdc.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDLIBS)

.PHONY: clean
clean:
	rm -f gimginfo gimgfixcmd gimgxor gimgunlock gimgch gimgextract cmdc *.o
