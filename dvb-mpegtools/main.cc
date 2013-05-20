/*
 *  dvb-mpegtools for the Siemens Fujitsu DVB PCI card
 *
 * Copyright (C) 2000, 2001 Marcus Metzler 
 *            for convergence integrated media GmbH
 * Copyright (C) 2002 Marcus Metzler 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 * 
 
 * The author can be reached at mocm@metzlerbros.de, 
 */


#include <cpptools.hh>

const int SIZE = 1024;
int fda_out, fdv_out;

void awrite(uint8_t const *buf,long int l)
{
        write(fda_out,buf,l);
}

void vwrite(uint8_t const *buf,long int l)
{
        write(fdv_out,buf,l);
}

void write_it(uint8_t *buf, int count, p2p *p)
{
        write(STDOUT_FILENO, buf, count);
}

void write_it2(uint8_t *buf, int count, void *p)
{
        write(STDOUT_FILENO, buf, count);
}

#define PROGS 29

char *prognames[PROGS+1] = { "streamtype",    // 0
                             "ts2pes", 	    // 1
                             "ps2vid",
                             "pes2aud_es",
                             "pes2aud",
                             "pes2vid_es",
                             "pes2vid",
                             "tspids",
                             "pes2ts",
                             "analyze",
                             "pes_demux", 	 // 10
                             "es_demux",
                             "ts_demux",
                             "ts_es_demux",
                             "pesplot",
                             "pes2ts2",
                             "pes_repack",
                             "split_mpg",
                             "cut_mpg",
                             "ts2ps",
                             "ts2es",                   // 20
                             "insert_pat_pmt",
                             "get_http",
                             "extract_pes",
                             "extract_pes_payload",
                             "change_aspect_1_1",
                             "change_aspect_4_3",
                             "change_aspect_16_9",
                             "change_aspect_221_1",	// 28
                             " "
                           };


enum { streamtype_,    // 0
       ts2pes_, 	    // 1
       ps2vid_,
       pes2aud_es_,
       pes2aud_,
       pes2vid_es_,
       pes2vid_,
       tspids_,
       pes2ts_,
       analyze_,
       pes_demux_, 	 // 10
       es_demux_,
       ts_demux_,
       ts_es_demux_,
       pesplot_,
       pes2ts2_,
       pes_repack_,
       split_mpg_,
       cut_mpg_,
       ts2ps_,
       ts2es_,            // 20
       insert_pat_pmt_,
       get_http_,
       extract_pes_,
       extract_pes_payload_,
       change_aspect_1_1_,
       change_aspect_4_3_,
       change_aspect_16_9_,
       change_aspect_221_1_,	// 28
       none_,
     };

void usage(int progn)
{
        switch(progn) {

        case streamtype_:
                cerr << "usage: streamtype <filename>" << endl;
                cerr << "or     streamtype < <filename> (or pipe)" << endl;
                break;

        case ts2pes_:
                cerr << "usage: ts2ps <filename> <PID1> <PID2>" << endl;
                cerr << "    or ts2ps <PID1> <PID2> < <filename> (or pipe)"
                << endl
                << " if pids = 0 the program searches for them"
                << endl;
                break;

        case ts2ps_:
                cerr << "usage: ts2ps <filename> <PID1> <PID2>" << endl;
                cerr << "    or ts2ps <PID1> <PID2> < <filename> (or pipe)"
                << endl
                << " if pids = 0 the program searches for them"
                << endl;
                break;

        case ts2es_:
                cerr << "usage: ts2es <filename> <PID>" << endl;
                cerr << "    or ts2ps <PID> < <filename> (or pipe)"
                << endl;
                break;

        case pes2aud_:
        case pes2vid_:
        case pes2aud_es_:
        case pes2vid_es_:
                cerr << "usage: " << prognames[progn] << " <filename>  [id]" << endl;
                cerr << "   or  " << prognames[progn] << " <filename>  [id]";
                cerr << "(or pipe)" << endl;
                break;

        case tspids_:
                cerr << "usage: tspids <filename>" << endl;
                break;

        case pes2ts_:
                cerr << "usage: pes2ts <filename>" << endl;
                cerr << "    or pes2ts < <filename> (or pipe)" << endl;
                break;

        case pes2ts2_:
                cerr << "usage: pes2ts2 <filename> <audio  PID> <video PID>"
                << endl;
                cerr << "   or  pes2ts2 < <filename> <audio  PID> <video PID>";
                cerr << " (or pipe)" << endl;
                break;

        case analyze_:
                cerr << "usage: analyze <filename>" << endl;
                cerr << "    or analyze < <filename> (or pipe)" << endl;
                break;

        case pes_demux_:
        case es_demux_:
                cerr << "usage: " << prognames[progn]
                << " <filename> <audioout> <videoout>" << endl;
                cerr << "   or  " << prognames[progn]
                << " <audioout> <videoout> < <filename>  (or pipe)"
                << endl;
                break;

        case ts_demux_:
                cerr << "usage: ts_demux <filename>" << endl;
                cerr << "   or  ts_demux < <filename>" << endl;
                break;

        case ts_es_demux_:
                cerr << "usage: ts_es_demux <filename> <audio_out> <video_out>"
                << endl;
                cerr << "   or  "
                << "ts_es_demux < <filename> <audio_out> <video_out>"
                << endl;
                break;

        case pesplot_:
                cerr << "usage: pesplot <filename>" << endl;
                cerr << "    or pesplot < <filename>" << endl;
                break;

        case pes_repack_:
                cerr << "usage: pes_repack <filename> <packet size>" << endl;
                cerr << "   or  pes_repack < <filename> <packet size>";
                cerr << "(or pipe)" << endl;
                break;

        case split_mpg_:
                cerr << "usage: split_mpg <filename> <size>" << endl;
                break;

        case cut_mpg_:
                cerr << "usage: cut_mpg <filename> <size>" << endl;
                break;

        case insert_pat_pmt_:
                cerr << "usage: insert_pat_pmt <filename>" << endl;
                cerr << "or     insert_pat_pmt < <filename> (or pipe)" << endl;
                break;

        case get_http_:
                cerr << "usage: get_http <URL>" << endl;
                break;

        case extract_pes_payload_:
                cerr << "usage: extract_pes_payload <filename> <ID in HEX>"
                << endl;
                cerr << "or extract_pes_payload <ID in HEX> < <filename>"
                << endl;
                break;

        case extract_pes_:
                cerr << "usage: extract_pes <filename> <ID in HEX>"
                << endl;
                cerr << "or extract_pes <ID in HEX> < <filename>"
                << endl;
                break;

        case change_aspect_1_1_:
        case change_aspect_4_3_:
        case change_aspect_16_9_:
        case change_aspect_221_1_:
                cerr << "usage: " << prognames[progn]
                << " <filename>" << endl;
                break;
        }
}


int main(int argc, char **argv)
{
        ifstream fin;
        int fd;
        int progn;
        int err = 0;

        for (progn = 0; progn < PROGS+1; progn ++)
                if (!strcmp(basename(argv[0]), prognames[progn]))
                        break;

        if (progn == PROGS+1 && argc > 1)
                for (progn = 0; progn < PROGS+1; progn ++)
                        if (!strcmp(basename(argv[1]), prognames[progn])) {
                                argc--;
                                argv++;
                                break;
                        }

        if (progn == PROGS+1) {
                for (progn = 0; progn < PROGS;progn++) {
                        usage(progn);
                        cerr << endl;
                }
                exit(1);
        }
        cerr << prognames[progn] << endl;

        if (argc >=2 && !strcmp(argv[1],"-h")) {
                usage(progn);
                exit(1);
        }

        if(argc <=2) {

                if (progn == get_http_) {
                        if (argc < 2) {
                                usage(progn);
                                exit(1);
                        }
                        int sock = http_open(argv[1]);

                        if (sock >= 0) {
                                int count = 1;
                                uint8_t buf[SIZE];
                                while (count > 0) {
                                        count = read(sock,buf,SIZE);
                                        write(STDOUT_FILENO,buf, count);
                                }

                        }
                        exit(0);
                }
                if (argc == 2 && progn != pes_repack_ && progn != ts2es_
                                && progn != extract_pes_
                                && progn != extract_pes_payload_) {
                        if ((fd = open(argv[1],O_RDONLY|O_LARGEFILE))< 0) {
                                perror(argv[1]);
                                exit(1);
                        }
                } else {
                        fd = STDIN_FILENO;
                }

                switch( progn ) {
                case streamtype_: {
                                char *streams[4] = { "PS_STREAM", "TS_STREAM"
                                                     , "PES_STREAM"};
                                char *tv_form[3] = {"DUNNO", "PAL", "NTSC"};

                                int f;
                                if (fd == STDIN_FILENO) {
                                        cout << streams[stream_type(cin)] << endl;
                                        f=tv_norm(cin);
                                } else {
                                        close(fd);
                                        if (argc < 2) {
                                                usage(progn);
                                                exit(1);
                                        }

                                        ifstream fin(argv[1]);
                                        cout << streams[stream_type(fin)] << endl;
                                        f=tv_norm(fin);
                                }
                                cout << tv_form[int(f&0x0F)] << endl;
                                cout << "MPEG" << int((f>>4)&0x0F) << endl;
                        }
                        break;

                case insert_pat_pmt_:
                        insert_pat_pmt( fd, STDOUT_FILENO);
                        break;

                case extract_pes_payload_: {
                                if (argc < 2) {
                                        usage(progn);
                                        exit(1);
                                }

                                istringstream arg(argv[1]);
                                int id;
                                arg >> hex >> id;
                                extract_from_pes(fd, STDOUT_FILENO, id, 1);
                                break;
                        }

                case extract_pes_: {
                                if (argc < 2) {
                                        usage(progn);
                                        exit(1);
                                }
                                istringstream arg(argv[1]);
                                int id;
                                arg >> hex >> id;
                                extract_from_pes(fd, STDOUT_FILENO, id, 0);
                                break;
                        }

                case pes2vid_:
                        extract_from_pes(fd, STDOUT_FILENO, 0xE0, 0);
                        break;

                case pes2aud_:
                        extract_from_pes(fd, STDOUT_FILENO, 0xC0, 0);
                        break;

                case pes2vid_es_:
                        extract_from_pes(fd, STDOUT_FILENO, 0xE0, 1);
                        break;

                case pes2aud_es_:
                        extract_from_pes(fd, STDOUT_FILENO, 0xC0, 1);
                        break;


                case pes2ts_: {
                                p2t_t p;
                                uint8_t buf[SIZE];

                                init_p2t(&p,NULL);
                                int count = 1;
                                while (count > 0) {
                                        count = read(fd,buf,SIZE);
                                        pes_to_ts(buf, count, 160, &p);
                                }
                        }
                        break;

                case ts_demux_:
                        cerr << "Opening stderr for audio" << endl;
                        cerr << "Opening stdout for video" << endl;

                        ts_demux(fd,STDOUT_FILENO,STDERR_FILENO,0,0,0);
                        break;

                case tspids_:
                        if (fd == STDIN_FILENO) {
                                TS_PIDS(cin,cout);
                        } else {
                                close(fd);
                                if (argc < 2) {
                                        usage(progn);
                                        exit(1);
                                }

                                ifstream fin(argv[1]);
                                TS_PIDS(cin,cout);
                        }
                        break;

                case analyze_:
                        if (fd == STDIN_FILENO) {
                                analyze(cin);
                        } else {
                                close(fd);
                                if (argc < 2) {
                                        usage(progn);
                                        exit(1);
                                }

                                ifstream fin(argv[1]);
                                analyze(fin);
                        }
                        break;

                case change_aspect_1_1_:
                        change_aspect( fd, STDOUT_FILENO, 0x10);
                        break;
                case change_aspect_4_3_:
                        change_aspect( fd, STDOUT_FILENO, 0x20);
                        break;
                case change_aspect_16_9_:
                        change_aspect( fd, STDOUT_FILENO, 0x30);
                        break;
                case change_aspect_221_1_:
                        change_aspect( fd, STDOUT_FILENO, 0x40);
                        break;

                case pes_repack_: {
                                uint8_t buf[SIZE];
                                int count = 1;
                                if (argc < 2) {
                                        usage(progn);
                                        exit(1);
                                }

                                int size = strtol(argv[1],(char **)NULL,0);
                                //			p2p p;
                                ipack p;

                                if (fd != STDIN_FILENO) {
                                        usage(progn);
                                        exit(1);
                                }
                                cerr << "Repacking int size " << size << endl;
                                //			init_p2p(&p, write_it, size);
                                init_ipack(&p, size, write_it2, 0);

                                while (count > 0) {
                                        count = read(fd,buf,SIZE);
                                        //				get_pes(buf,count,&p,pes_repack);
                                        instant_repack(buf,count,&p);
                                }
                        }
                        break;

                case ts2es_:
                        if (fd != STDIN_FILENO || argc < 2) {
                                usage(progn);
                                exit(1);
                        }
                        ts2es(fd, strtol(argv[1],(char **)NULL,0));
                        break;

                case pesplot_: {
                                PES_Packet pes;
                                long int apts=0;
                                long int vpts=0;

                                if (fd == STDIN_FILENO) {
                                        cerr << "Can't read from stdin" << endl;
                                        exit(1);
                                }
                                close(fd);
                                if (argc < 2) {
                                        usage(progn);
                                        exit(1);
                                }


                                ifstream fin(argv[1]);

                                while(!fin.eof()) {
                                        streampos pos=fin.tellg();
                                        fin >> pes;
                                        if(pes.has_pts()) {
                                                switch(pes.Stream_ID()) {
                                                case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                                                        apts = ntohl(*((unsigned int *)
                                                                       pes.P()->pts));
                                                        break;
                                                case VIDEO_STREAM_S ... VIDEO_STREAM_E:
                                                        vpts = ntohl(*((unsigned int *)
                                                                       pes.P()->pts));
                                                        break;
                                                }
                                                if (apts && vpts)
                                                        cout << pos << " "
                                                        << apts << " " << vpts
                                                        << endl;
                                        }
                                }
                        }
                        break;

                default:
                        err++;
                        break;
                }
        } else
                err++;


        if (argc == 3 || argc == 4) {
                int i=0;
                int pes=0;
                if (argc == 3) {
                        switch (progn) {
                        case pes2ts2_: {
                                        pes_to_ts2( STDIN_FILENO, STDOUT_FILENO,
                                                    strtol(argv[1],(char **)NULL, 0),
                                                    strtol(argv[2],(char **)NULL, 0));
                                        cerr << endl;
                                }
                                break;

                        case extract_pes_payload_:
                        case pes2vid_es_:
                        case pes2aud_es_:
                                pes = 1;
                        case extract_pes_:
                        case pes2vid_:
                        case pes2aud_: {
                                        istringstream arg(argv[2]);
                                        int id = -1;
                                        if ((fd = open(argv[1],O_RDONLY|O_LARGEFILE))
                                                        <0) {
                                                perror(argv[1]);
                                                exit(1);
                                        }
                                        arg >> hex >> id;
                                        switch(progn) {
                                        case pes2vid_es_:
                                        case pes2vid_:
                                                switch(id) {
                                                case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                                                case VIDEO_STREAM_S ... VIDEO_STREAM_E:
                                                        break;

                                                default:
                                                        cerr << "Not a valid video id" << endl;
                                                        exit(1);
                                                }
                                                break;

                                        case pes2aud_:
                                        case pes2aud_es_:
                                                switch(id) {
                                                case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                                                        break;

                                                default:
                                                        cerr << "Not a valid audio id" << endl;
                                                }
                                                break;

                                        default:
                                                break;

                                        }
                                        arg >> hex >> id;
                                        extract_from_pes(fd, STDOUT_FILENO, id, pes);
                                        break;
                                }

                        case ts2es_:
                                if ((fd = open(argv[1],O_RDONLY|O_LARGEFILE)) <0) {
                                        perror(argv[1]);
                                        exit(1);
                                }
                                ts2es(fd, strtol(argv[2],(char **)NULL,0));
                                break;

                        case pes_repack_: {
                                        uint8_t buf[SIZE];
                                        int count = 1;
                                        int size = strtol(argv[2],(char **)NULL,0);
                                        //				p2p p;
                                        ipack p;

                                        if (argc > 3) {
                                                usage(progn);
                                                exit(1);
                                        }
                                        if ((fd = open(argv[1],O_RDONLY|O_LARGEFILE))
                                                        <0) {
                                                perror(argv[1]);
                                                exit(1);
                                        }
                                        if (fd != STDIN_FILENO)
                                                cerr << "Reading " << argv[1] << endl;
                                        else
                                                exit(1);

                                        cerr << "Repacking int size " << size << endl;
                                        //				init_p2p(&p, write_it, size);
                                        init_ipack(&p, size, write_it2, 0);

                                        while (count > 0) {
                                                count = read(fd,buf,SIZE);
                                                instant_repack(buf,count,&p);
                                        }
                                }
                                return 0;

                        }
                }


                if (argc == 4) {
                        if ((fd = open(argv[1],O_RDONLY|O_LARGEFILE)) < 0) {
                                perror(argv[1]);
                                exit(1);
                        }
                } else {
                        i=1;
                        fd = STDIN_FILENO;
                }

                switch (progn) {

                case ts2pes_:
                        ts_to_pes(fd,strtol(argv[2-i],(char **)NULL, 0),
                                  strtol(argv[3-i],(char **)NULL, 0),0);
                        break;

                case ts2ps_:
                        ts_to_pes(fd,strtol(argv[2-i],(char **)NULL, 0),
                                  strtol(argv[3-i],(char **)NULL, 0),1);
                        break;

                case pes_demux_: {
                                fda_out = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);
                                fdv_out = open(argv[3],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);
                                if(fda_out < 0 || fdv_out < 0) {
                                        cerr << "Error opening output files" << endl;
                                        exit(1);
                                }
                                cerr << "Opening " << argv[2] << " for audio" << endl;
                                cerr << "Opening " << argv[3] << " for video" << endl;

                                pes_dmx( fd, fda_out, fdv_out, 0);
                        }
                        break;

                case ts_demux_: {
                                fda_out = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);
                                fdv_out = open(argv[3],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);
                                if(fda_out < 0 || fdv_out < 0) {
                                        cerr << "Error opening output files" << endl;
                                        exit(1);
                                }
                                cerr << "Opening " << argv[2] << " for audio" << endl;
                                cerr << "Opening " << argv[3] << " for video" << endl;

                                ts_demux(fd,fdv_out,fda_out,0,0,0);
                        }
                        break;

                case ts_es_demux_: {
                                fda_out = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);
                                fdv_out = open(argv[3],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);
                                if(fda_out < 0 || fdv_out < 0) {
                                        cerr << "Error opening output files" << endl;
                                        exit(1);
                                }
                                cerr << "Opening " << argv[2] << " for audio" << endl;
                                cerr << "Opening " << argv[3] << " for video" << endl;

                                ts_demux(fd, fdv_out, fda_out, 0, 0, 1);
                        }
                        break;


                case es_demux_: {
                                fda_out = open(argv[2],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);

                                fdv_out = open(argv[3],O_WRONLY|O_CREAT|O_TRUNC
                                               |O_LARGEFILE,
                                               S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                               S_IROTH|S_IWOTH);
                                if(fda_out < 0 || fdv_out < 0) {
                                        cerr << "Error opening output files" << endl;
                                        exit(1);
                                }
                                cerr << "Opening " << argv[2] << " for audio" << endl;
                                cerr << "Opening " << argv[3] << " for video" << endl;

                                cerr << "VPTS - APTS = "
                                <<	pes_dmx(fd, fda_out, fdv_out, 1)/90000.
                                //<< es_demux(fd, fdv_out, fda_out)/90000.
                                << "s" << endl;
                        }
                        break;

                case pes2ts2_: {
                                pes_to_ts2( fd, STDOUT_FILENO,
                                            strtol(argv[2-i],(char **)NULL, 0),
                                            strtol(argv[3-i],(char **)NULL, 0));
                                cerr << endl;
                        }
                        break;

                case split_mpg_:
                        close(fd);
                        split_mpg(argv[1],strtol(argv[2],(char **)NULL,0)*1024*1024);
                        break;

                case cut_mpg_:
                        close(fd);
                        cut_mpg(argv[1],strtol(argv[2],(char **)NULL,0));
                        break;
                }
        } else
                err++;
        if (err == 2)
                usage(progn);
}
