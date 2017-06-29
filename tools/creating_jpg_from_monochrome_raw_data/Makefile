# Dalam Makefile ini, kita bikin jpec_ori. File jpec_ori.c sebenarnya berasal dari main.c
# yang dimodifikasi untuk menggunakan opencv2 (karena opencv punya masalah dengan cvRound)
rm=rm -f
CC=cc
GG=g++
AR=ar cq
RANLIB=ranlib
LIBNAME=libjpec.a
CLINAME=jpec_ori 
MYPROG=encjpgfromra1
# LIB_BUILD_MODE: release or debug
LIB_BUILD_MODE=release

INCLUDES= -Isrc -Iinclude

DEFINES= $(INCLUDES)
_release_cflags= -Os -DNDEBUG
_debug_cflags= -g -O0
CFLAGS= $(DEFINES) -std=c99 -Wall -Werror
CFLAGS+= $(_$(LIB_BUILD_MODE)_cflags)

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

%.c.o:
	$(CC) $(CFLAGS) -c $*.c

all: $(CLINAME) $(MYPROG) $(LIBNAME)

$(LIBNAME): $(OBJS)
	$(rm) $@
	$(AR) $@ $(OBJS)
	$(RANLIB) $@

$(CLINAME): $(LIBNAME) jpec_ori.c
	#$(CC) -o $@ jpec_ori.c -std=c99 `pkg-config opencv --cflags --libs` -Iinclude -L. -l$@
	$(GG) -o $@ jpec_ori.c `pkg-config opencv --cflags --libs` -Iinclude -L. -ljpec
	
$(MYPROG): $(LIBNSME) encjpgfromra1.c
	$(CC) -o $@ encjpgfromra1.c -Iinclude -L. -ljpec

clean:
	$(rm) $(OBJS) $(CLINAME) $(MYPROG) $(LIBNAME)
