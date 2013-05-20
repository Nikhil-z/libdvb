
include ../config.mk

LIBS=-L../ 
DVB_LIBS=libdvb.a libdvbci.a
HEADERS=DVB.hh ci.hh devices.hh channel.h
SOURCES=Makefile $(HEADERS) DVB.cc ci.cc devices.cc OSD.h OSD.c osd.hh\
	merge.cc conv.cc satscan.cc cam_set.cc cam_test.cc quickscan.cc cam_menu.cc
RESOURCES=README astrarc hotbirdrc

main: $(TEST_PROGS)

dump_TS: dump_TS.cc ../libdvb/libdvb.a ../include/DVB.hh
	$(CXX) $(LIBS) $(INCLUDES) $(CFLAGS) -o dump_TS dump_TS.cc -ldvb -ldvbmpegtools

merge_dvb: merge.cc ../libdvb/libdvb.a ../include/DVB.hh
	$(CXX) $(LIBS) $(INCLUDES) $(CFLAGS) -o merge_dvb merge.cc -ldvb 

conv: conv.cc ../libdvb/libdvb.a ../include/DVB.hh
	$(CXX) $(LIBS) $(INCLUDES) $(CFLAGS) -o conv conv.cc -ldvb

satscan: satscan.cc ../libdvb/libdvb.a ../include/DVB.hh
	$(CXX) $(LIBS) $(INCLUDES) $(CFLAGS) -o satscan satscan.cc -ldvb

quickscan: quickscan.cc ../libdvb/libdvb.a ../include/DVB.hh
	$(CXX) $(LIBS) $(INCLUDES) $(CFLAGS) -o quickscan quickscan.cc -ldvb

cam_set: cam_set.cc cam_menu.hh ../libdvbci/libdvbci.a ../include/ci.hh
	$(CXX) $(LIBS) $(INCLUDES) $(CFLAGS) -o cam_set cam_set.cc -ldvbci

cam_test: cam_test.cc ../libdvb/libdvb.a ../include/DVB.hh ../include/ci.hh
	$(CXX) $(LIBS) $(INCLUDES) -o cam_test cam_test.cc -ldvb -ldvbmpegtools

cam_menu: cam_menu.cc cam_menu.hh ../libdvb/libdvb.a ../include/DVB.hh
	$(CXX) $(LIBS) $(INCLUDES) -o cam_menu cam_menu.cc -ldvb

status_check: status_check.c
	$(CC) $(INCLUDES) $(CFLAGS) -o status_check status_check.c

.c.o: 
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

.cc.o: 
	$(CXX) $(INCLUDES) $(CFLAGS) -c $<

clean:
	-rm -f *.o $(TEST_PROGS) *~

