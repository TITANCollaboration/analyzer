# currently set up for packages/old-versions/midas-unknown-grsmid00/linux-m32

DRV_DIR         = $(MIDASSYS)/drivers/bus
INC_DIR 	= $(MIDASSYS)/include
LIB_DIR 	= $(MIDASSYS)/linux/lib
#LIB_DIR 	= $(MIDASSYS)/linux-m32/lib

OSFLAGS = -DOS_LINUX -Dextname -DUSE_ROODY
CFLAGS = -O -Wall
#LIBS = -lm -lz -lutil -lnsl -lpthread # Now need -lrt for some reason ???
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt

LIB = $(LIB_DIR)/libmidas.a

CC = g++
CXX = g++
CFLAGS += -g -I$(INC_DIR) -I$(DRV_DIR) -I$(VME_DIR)/include
LDFLAGS +=
LDFEFLAGS += -L$(VME_DIR)/lib -lvme

MODULES 	= angrif.o anmdpp.o histogram.o web_server.o

ROOTCFLAGS := $(shell  $(ROOTSYS)/bin/root-config --cflags)
ROOTCFLAGS += -DHAVE_ROOT -DUSE_ROOT
ROOTLIBS   := $(shell  $(ROOTSYS)/bin/root-config --libs)
ROOTLIBS   += -lThread

#analyzer: $(LIB64_DIR)/rmana.o $(LIB64_DIR)/libmidas.a analyzer.o anmdpp.o
#        $(CXX) $(CFLAGS) -o $@ $^ $(ROOTLIBS) $(LIBS)
        
        
all: analyzer

analyzer: $(LIB) $(LIB_DIR)/mana.o analyzer.o $(MODULES) Makefile
	$(CC) $(ROOTCFLAGS) $(CFLAGS) -o $@ $(LIB_DIR)/mana.o analyzer.o $(MODULES) \
	$(LIB) $(LDFLAGS) $(ROOTLIBS) $(LIBS)

analyzer.o: analyzer.c
	$(CC) $(USERFLAGS) $(CFLAGS) $(ROOTCFLAGS) $(OSFLAGS) -o $@ -c $<

angrif.o: angrif.c web_server.h histogram.h
	$(CC) $(USERFLAGS) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

anmdpp.o: anmdpp.c web_server.h histogram.h
	$(CC) $(USERFLAGS) $(CFLAGS) $(ROOTCFLAGS) $(OSFLAGS) -o $@ -c $<

web_server.o: web_server.c web_server.h histogram.h
	$(CC) $(USERFLAGS) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

histogram.o: histogram.c histogram.h
	$(CC) $(USERFLAGS) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

clean::
	rm -f *.o *~ \#* $(FEMODULES) $(MODULES)

#end file
