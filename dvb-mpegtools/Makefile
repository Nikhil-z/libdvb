
include ../config.mk

DCFLAGS =  $(CFLAGS) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
LIBS   = -L../ -ldvbmpegtools
MFLAG  = -M
MPLEXOBS = buffer.o inits.o interact.o systems.o domplex.o inptstrm.o \
	multplex.o timecode.o mplex.o bits.o

SRC  = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
CSRC = $(wildcard *.cc) 
COBJ = $(notdir $(SRC:.c=.o)) 
LINKS = streamtype ts2pes ps2vid pes2aud_es pes2aud pes2vid_es pes2vid \
	tspids pes2ts analyze pes_demux es_demux ts_demux ts_es_demux \
	pesplot pes2ts2 pes_repack split_mpg cut_mpg ts2ps ts2es\
	insert_pat_pmt get_http extract_pes extract_pes_payload\
	change_aspect_1_1 change_aspect_4_3 change_aspect_16_9 \
	change_aspect_221_1

.PHONY: depend clean install uninstall


all: dvb-mpegtools_main dvb-mplex links audiofilter dvbaudio show_dvb_image

links: dvb-mpegtools_main
	for f in $(LINKS); do\
		ln -sf dvb-mpegtools_main $$f ;\
	done

clean:
	- rm -f *.o .depend dvb-mpegtools_main dvb-mplex ctest *~ $(LINKS) ctest audiofilter show_dvb_image\
	dvbaudio

dvb-mpegtools_main: main.o
	$(CXX) main.o $(LIBS) -o $@

ctest: mainc.o 
	$(CC) $(COBJ) -o $@

dvb-mplex: $(MPLEXOBS)
	$(CXX) $(MPLEXOBS) $(LIBS) -o $@

dvbaudio: dvbaudio.o
	$(CXX) dvbaudio.o $(LIBS) -ldvb -o $@

audiofilter: audiofilter.o
	$(CXX) audiofilter.o -o $@

%.o:    %.cc 
	$(CXX) -c $(DCFLAGS) $(INCLUDES) $(DEFINES) $<

%.o:    %.cpp
	$(CXX) -c $(DCFLAGS) $(INCLUDES) $(DEFINES) $<

%.o:    %.c 
	$(CC) -c $(DCFLAGS) $(INCLUDES) $(DEFINES) $<

install: dvb-mpegtools_main
	install -D -m 755 dvb-mplex $(DESTDIR)$(PREFIX)/bin/dvb-mplex
	install -m 755 audiofilter $(DESTDIR)$(PREFIX)/bin/
	install -m 755 dvb-mpegtools_main $(DESTDIR)$(PREFIX)/bin/
	install -m 755 dvbaudio $(DESTDIR)$(PREFIX)/bin/
	install -m 755 show_dvb_image $(DESTDIR)$(PREFIX)/bin/
	install -m 755 dia $(DESTDIR)$(PREFIX)/bin/
	for f in $(LINKS); do\
		(cd $(DESTDIR)$(PREFIX)/bin && ln -s -f dvb-mpegtools_main $$f) ;\
	done

uninstall:
	for f in $(LINKS); do rm -f $(DESTDIR)$(PREFIX)/bin/$$f;done
	rm -f $(DESTDIR)$(PREFIX)/bin/dvb-mpegtools_main
	rm -f $(DESTDIR)$(PREFIX)/bin/dvbaudio
	rm -f $(DESTDIR)$(PREFIX)/bin/audiofilter
	rm -f $(DESTDIR)$(PREFIX)/lib/libdvbmpegtools

.depend: 
	$(CXX) $(DEFINES) $(MFLAG) $(SRC) $(CSRC) $(CPPSRC) $(INCLUDES)> .depend



include .depend
