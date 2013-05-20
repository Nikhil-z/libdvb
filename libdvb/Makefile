
include ../config.mk

OBJS=devices.o DVB.o OSD.o 
LIBS=-L./ -lpthread
DVB_LIBS=libdvb.a

main: $(DVB_LIBS) 

libdvb.a: $(OBJS)
	ar -rcs libdvb.a $(OBJS) 

DVB.o: DVB.cc ../include/DVB.hh
	$(CXX) $(INCLUDES) $(CFLAGS) -c $<

devices.o: devices.cc ../include/devices.hh
	$(CXX) $(INCLUDES) $(CFLAGS) -c $<

.c.o: 
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

.cc.o: 
	$(CXX) $(INCLUDES) $(CFLAGS) -c $<

clean:
	-rm -f *.o $(DVB_LIBS) *~
