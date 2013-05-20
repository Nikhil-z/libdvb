
SOURCEDIRS=include libdvb libdvbci libdvbmpeg sample_progs dvb-mpegtools samplerc 
DVB-MPEGTOOLS= dvb-mpegtools_main dvb-mplex audiofilter dvbaudio
DVB_LIBS=libdvb.a libdvbci.a libdvbmpegtools.a
HEADERS=include/DVB.hh include/ci.hh include/devices.hh include/channel.h include/ctools.h \
	include/transform.h include/ringbuffy.h include/cpptools.hh include/OSD.h \
	include/osd.hh

RESOURCES=README
include config.mk



main: $(DVB_LIBS) $(TEST_PROGS) $(DVB-MPEGTOOLS)

install: $(DVB_LIBS) $(HEADERS)
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(DVB_LIBS) $(DESTDIR)$(PREFIX)/lib/
	mkdir -p $(DESTDIR)$(PREFIX)/include
	install -m 644 $(HEADERS) $(DESTDIR)$(PREFIX)/include/
	make -C dvb-mpegtools install

uninstall: 
	for i in $(DVB_LIBS); do rm -f $(DESTDIR)$(PREFIX)/lib/$$i;done
	for i in $(HEADERS); do rm -f $(DESTDIR)$(PREFIX)/$$i;done
	make -C dvb-mpegtools uninstall

libdvb.a: libdvb/libdvb.a
	cp libdvb/libdvb.a .

libdvbci.a: libdvbci/libdvbci.a
	cp libdvbci/libdvbci.a .

libdvbmpegtools.a: libdvbmpeg/libdvbmpegtools.a
	cp libdvbmpeg/libdvbmpegtools.a .

libdvb/libdvb.a:
	make -C libdvb main

libdvbci/libdvbci.a:
	make -C libdvbci main

libdvbmpeg/libdvbmpegtools.a:
	make -C libdvbmpeg libdvbmpegtools.a

merge_dvb: libdvb.a sample_progs/merge.cc
conv:      libdvb.a sample_progs/conv.cc
satscan:   libdvb.a sample_progs/satscan.cc
quickscan:   libdvb.a sample_progs/quickscan.cc
cam_set:   libdvbci.a sample_progs/cam_set.cc
cam_test:  libdvb.a libdvbmpegtools.a sample_progs/cam_test.cc
	make -C sample_progs main
	for f in $(TEST_PROGS); do cp sample_progs/$$f . ; done

dvb-mpegtools_main: libdvbmpegtools.a dvb-mpegtools/main.cc
dvb-mplex:          libdvbmpegtools.a dvb-mpegtools/mplex.cpp
audiofilter:        dvb-mpegtools/audiofilter.c
dvbaudio:           libdvb.a libdvbmpegtools.a dvb-mpegtools/dvbaudio.cc
	make -C dvb-mpegtools all
	for f in $(DVB-MPEGTOOLS); do cp dvb-mpegtools/$$f . ; done

dvbs:  libdvb.a libdvbmpegtools.a dvbserver/dvbs_main.cc
	make -C dvbserver  dvbs_main


clean:
	-rm -f libdvb-$(VERSION).tar.gz $(DVB_LIBS) $(TEST_PROGS) $(DVB-MPEGTOOLS) *~
	make -C libdvb clean
	make -C libdvbci clean
	make -C libdvbmpeg clean
	make -C sample_progs clean
	make -C dvb-mpegtools clean
	rm include/*~

dist:	
	mkdir libdvb-$(VERSION)
	for f in $(SOURCEDIRS); do mkdir libdvb-$(VERSION)/$$f;done
	cp -r $(SOURCEDIRS) libdvb-$(VERSION)
	for f in $(SOURCEDIRS); do rm -rf libdvb-$(VERSION)/$$f/CVS;done
	cp COPYING README Makefile config.mk libdvb-$(VERSION)
	tar zcf libdvb-$(VERSION).tar.gz libdvb-$(VERSION)
	rm -rf libdvb-$(VERSION)
