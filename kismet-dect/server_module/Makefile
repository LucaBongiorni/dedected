# Kismet wireless sniffer

# Installation goes here
KIS_DEST_DIR ?= /usr/local
# You will need kismet newcore sources
KIS_SRC_DIR ?= /usr/src/kismet-newcore
KIS_INC_DIR ?= $(KIS_SRC_DIR)

HOME	= .
top_builddir = $(HOME)
CXX	= g++
CC	= gcc
LD	= g++
LDFLAGS	= -shared -rdynamic 
LIBS	= -lstdc++
CFLAGS 	= -I/usr/include -I$(KIS_INC_DIR) -g -fPIC

PLUGIN_NAME = dect_srv
LIBOBJS = $(PLUGIN_NAME)plugin.o
LIBOUT	= $(PLUGIN_NAME)plugin.so

all:	$(LIBOUT) 

$(LIBOUT):	$(LIBOBJS)
	$(LD) $(LDFLAGS) $(LIBOBJS) -o $(LIBOUT) $(LIBS)

clean:
	@-rm -f *.o
	@-rm -f *.so

install: $(LIBOUT)
	install $^ $(KIS_DEST_DIR)/lib/kismet

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $@ 

.cc.o:
	$(CXX) $(CFLAGS) -c $*.cc -o $@ 

.SUFFIXES: .c .cc .o
