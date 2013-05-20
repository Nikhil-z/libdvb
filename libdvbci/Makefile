
include ../config.mk

CI_OBJS=ci.o ci2.o
LIBS=-L./ -lpthread
DVB_LIBS=libdvbci.a

main: $(DVB_LIBS)

libdvbci.a: $(CI_OBJS)
	ar -rcs libdvbci.a $(CI_OBJS) 

ci.o: ci.cc ../include/ci.hh
	$(CXX) $(INCLUDES) $(CFLAGS) -c $<

.c.o: 
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

.cc.o: 
	$(CXX) $(INCLUDES) $(CFLAGS) -c $<

clean:
	-rm -f *.o $(DVB_LIBS) *~
