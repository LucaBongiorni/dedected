# Kismet wireless sniffer

# Installation goes here
KIS_DEST_DIR ?= /usr/local
# You will need kismet newcore sources
KIS_SRC_DIR ?= /usr/src/kismet
KIS_INC_DIR ?= $(KIS_SRC_DIR)

HOME	= .
CXX	= g++
CC	= gcc
LD	= g++
LDFLAGS	= -shared -rdynamic
LIBS	= -lstdc++ 
CFLAGS	= -fPIC
CXXFLAGS = -fPIC -pthread 
CPPFLAGS = -I/usr/include -I$(KIS_INC_DIR) -I.

PLUGIN_NAME = dect_cli
LIBOBJS = $(PLUGIN_NAME)plugin.o
LIBOUT	= $(PLUGIN_NAME)plugin.so

$(LIBOUT):	$(LIBOBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	@-rm -f *.o
	@-rm -f *.so

install: $(LIBOUT)
	install -D $^ $(KIS_DEST_DIR)/lib/kismet_client/$^

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $@ 

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $*.cc -o $@ 

.SUFFIXES: .c .cc .o
