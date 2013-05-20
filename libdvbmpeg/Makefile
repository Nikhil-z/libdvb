
include ../config.mk

DCFLAGS =  $(CFLAGS) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
MFLAG  = -M
OBJS = ctools.o ringbuffy.o transform.o cpptools.o
SRC  = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
CSRC = $(wildcard *.cc) 
INCS = -I../include

all: libdvbmpegtools.a

.PHONY: depend clean install uninstall

clean:
	- rm -f *.o  *~ *.a .depend

libdvbmpegtools.a: $(OBJS)
	ar -rcs libdvbmpegtools.a $(OBJS) 

%.o:    %.cc 
	$(CXX) -c $(DCFLAGS) $(INCLUDES) $(DEFINES) $<

%.o:    %.cpp
	$(CXX) -c $(DCFLAGS) $(INCLUDES) $(DEFINES) $<

%.o:    %.c 
	$(CC) -c $(DCFLAGS) $(INCLUDES) $(DEFINES) $<

.depend: 
	$(CXX) $(DEFINES) $(MFLAG) $(SRC) $(CSRC) $(CPPSRC) $(INCLUDES)> .depend



include .depend
