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

#define MPEGMAIN
#include <getopt.h>
#include "main.h"
#include "consts.h"
#include <cpptools.hh>


void SetXSVCDMplex()
{
        /* multiplex stuff */
        cerr << "Setting up for XSVCD mux" << endl;
        sectors_delay = 0;
        video_delay_ms = 180;
        audio_delay_ms = 180;
        audio1_delay_ms = 180;
        sector_size = 2324;
        packets_per_pack = 1;
        init_audio_buffer_size = 4;
        init_audio1_buffer_size = 4;
        init_video_buffer_size = 230;
        always_sys_header = FALSE;
        mplex_type = MPEG_SVCD;
        mplex_pulldown_flag = PULLDOWN_AUTO;
        vcd_audio_pad = FALSE;
        user_mux_rate = 0;
        align_sequence_headers = 1;
        put_private2 = 0;
        frame_timestamps = TIMESTAMPS_ALL;
        VBR_multiplex = TRUE;
        use_computed_bitrate = COMPBITRATE_MAX;
        write_pec = 1;
        mux_SVCD_scan_offsets = 1;
        max_file_size = 0;
        mux_start_time = 0;
        mux_stop_time = 0;
        reset_clocks = 1;
        write_end_codes = 1;
        set_broken_link = 1;
}

void SetMPEG2Mplex()
{
        /* multiplex stuff */
        cerr << "Setting up for MPEG2 mux" << endl;
        sectors_delay = 0;
        video_delay_ms = 180;
        audio_delay_ms = 180;
        audio1_delay_ms = 180;
        sector_size = 2048;
        packets_per_pack = 1;
        init_audio_buffer_size = 4;
        init_audio1_buffer_size = 4;
        init_video_buffer_size = 224;
        always_sys_header = FALSE;
        mplex_type = MPEG_MPEG2;
        mplex_pulldown_flag = PULLDOWN_AUTO;
        vcd_audio_pad = FALSE;
        user_mux_rate = 0;
        align_sequence_headers = 0;
        put_private2 = 0;
        frame_timestamps = TIMESTAMPS_ALL;
        VBR_multiplex = TRUE;
        use_computed_bitrate = COMPBITRATE_MAX;
        write_pec = 1;
        mux_SVCD_scan_offsets = 0;
        max_file_size = 0;
        mux_start_time = 0;
        mux_stop_time = 0;
        reset_clocks = 1;
        write_end_codes = 1;
        set_broken_link = 1;
}

void SetDVDMplex()
{
        /* multiplex stuff */
        cerr << "Setting up for DVD mux" << endl;
        sectors_delay = 0;
        video_delay_ms = 180;
        audio_delay_ms = 180;
        audio1_delay_ms = 180;
        sector_size = 2048;
        packets_per_pack = 1;
        init_audio_buffer_size = 4;
        init_audio1_buffer_size = 4;
        init_video_buffer_size = 232;
        always_sys_header = FALSE;
        use_computed_bitrate = COMPBITRATE_MAX;
        mplex_type = MPEG_DVD;
        mplex_pulldown_flag = PULLDOWN_AUTO;
        vcd_audio_pad = FALSE;
        user_mux_rate = 25200;
        align_sequence_headers = TRUE;
        put_private2 = TRUE;
        frame_timestamps = TIMESTAMPS_IONLY;
        VBR_multiplex = TRUE;
        write_pec = 1;
        mux_SVCD_scan_offsets = 0;
        max_file_size = 0;
        mux_start_time = 0;
        mux_stop_time = 0;
        reset_clocks = 0;
        write_end_codes = 0;
        set_broken_link = 0;
}

void SetSVCDMplex()
{
        /* multiplex stuff */
        cerr << "Setting up for SVCD mux" << endl;
        sectors_delay = 0;
        video_delay_ms = 180;
        audio_delay_ms = 180;
        audio1_delay_ms = 180;
        sector_size = SVCD_SECTOR_SIZE;
        packets_per_pack = 1;
        init_audio_buffer_size = 4;
        init_audio1_buffer_size = 4;
        init_video_buffer_size = 230;
        always_sys_header = FALSE;
        use_computed_bitrate = COMPBITRATE_MAX;
        mplex_type = MPEG_SVCD;
        mplex_pulldown_flag = PULLDOWN_AUTO;
        vcd_audio_pad = FALSE;
        user_mux_rate = 6972;
        align_sequence_headers = 1;
        put_private2 = 0;
        frame_timestamps = TIMESTAMPS_ALL;
        VBR_multiplex = TRUE;
        write_pec = 1;
        mux_SVCD_scan_offsets = 1;
        max_file_size = 0;
        mux_start_time = 0;
        mux_stop_time = 0;
        reset_clocks = 1;
        write_end_codes = 1;
        set_broken_link = 1;
}

void SetMPEG1Mplex()
{
        /* multiplex stuff */
        cerr << "Setting up for MPEG1 mux" << endl;
        sectors_delay = 0;
        video_delay_ms = 180;
        audio_delay_ms = 180;
        audio1_delay_ms = 180;
        sector_size = 2048;
        packets_per_pack = 1;
        init_audio_buffer_size = 4;
        init_audio1_buffer_size = 4;
        init_video_buffer_size = 46;
        always_sys_header = FALSE;
        mplex_type = MPEG_MPEG1;
        mplex_pulldown_flag = PULLDOWN_NONE;
        vcd_audio_pad = FALSE;
        user_mux_rate = 0;
        align_sequence_headers = 0;
        put_private2 = 0;
        frame_timestamps = TIMESTAMPS_ALL;
        VBR_multiplex = FALSE;
        use_computed_bitrate = COMPBITRATE_MAX;
        write_pec = 1;
        mux_SVCD_scan_offsets = 0;
        max_file_size = 0;
        mux_start_time = 0;
        mux_stop_time = 0;
        reset_clocks = 1;
        write_end_codes = 1;
        set_broken_link = 1;
}


void SetVCDMplex()
{
        /* multiplex stuff */
        cerr << "Setting up for VCD mux" << endl;
        sectors_delay = 400;
        video_delay_ms = 344;
        audio_delay_ms = 344;
        audio1_delay_ms = 344;
        sector_size = VIDEOCD_SECTOR_SIZE;
        packets_per_pack = 1;
        init_audio_buffer_size = 4;
        init_audio1_buffer_size = 4;
        init_video_buffer_size = 46;
        always_sys_header = FALSE;
        use_computed_bitrate = COMPBITRATE_NONE;
        mplex_type = MPEG_VCD;
        mplex_pulldown_flag = PULLDOWN_NONE;
        vcd_audio_pad = FALSE;
        user_mux_rate = 3486;
        align_sequence_headers = 0;
        put_private2 = 0;
        frame_timestamps = TIMESTAMPS_ALL;
        VBR_multiplex = 0;
        write_pec = 1;
        mux_SVCD_scan_offsets = 0;
        max_file_size = 0;
        mux_start_time = 0;
        mux_stop_time = 0;
        reset_clocks = 1;
        write_end_codes = 1;
        set_broken_link = 1;
}


void SetXVCDMplex()
{
        /* multiplex stuff */
        cerr << "Setting up for XVCD mux" << endl;
        sectors_delay = 400;
        video_delay_ms = 344;
        audio_delay_ms = 344;
        audio1_delay_ms = 344;
        sector_size = VIDEOCD_SECTOR_SIZE;
        packets_per_pack = 1;
        init_audio_buffer_size = 4;
        init_audio1_buffer_size = 4;
        init_video_buffer_size = 46;
        always_sys_header = FALSE;
        use_computed_bitrate = COMPBITRATE_MAX;
        mplex_type = MPEG_MPEG1;
        mplex_pulldown_flag = PULLDOWN_NONE;
        vcd_audio_pad = FALSE;
        user_mux_rate = 0;
        align_sequence_headers = 0;
        put_private2 = 0;
        frame_timestamps = TIMESTAMPS_ALL;
        VBR_multiplex = 1;
        write_pec = 1;
        mux_SVCD_scan_offsets = 0;
        max_file_size = 0;
        mux_start_time = 0;
        mux_stop_time = 0;
        reset_clocks = 1;
        write_end_codes = 1;
        set_broken_link = 1;
}


void usage(char *progname)
{
        printf ("usage: %s [options] <input files>\n\n",progname);
        printf ("options:\n");
        printf ("  --help,             -h:  print help message\n");
        printf ("  --type,             -t:  set output type (MPEG2, DVD, SVCD (default), MPEG1, VCD, XSVCD, XVCD)\n");
        printf ("  --of,               -o:  set output file\n");
        printf ("  --input_stream,     -i:  set input stream type (PS_STREAM (default), TS_STREAM, TS_AC3_STREAM, ES_STREAM (needs 2 or 3 input files <audio1> [ <audio2> ] <video>))\n");
        printf ("  --temp_dir,         -d:  directory for temporary demux files\n");
        printf ("  --audio_delay,      -a:  set audio delay in ms\n");
        printf ("  --video_delay,      -v:  set video delay in ms\n");
        printf ("  --audio_pid,        -q:  audio PID for TS stream (0 = auto)\n");
        printf ("  --video_pid,        -p:  audio PID for TS stream (0 = auto)\n");
        printf ("  --ac3_id,           -c:  ID of AC3 audio for demux(0 = 1st tream)\n");
        exit(1);
}

int main(int argc, char **argv)
{
        int fda_out=0, fdv_out=0, fd_in=0;
        int64_t ptsdiff=0;
        int c;
        char *filename = NULL;
        char *type = "SVCD";
        char *inpt = "PS_STREAM";
        int adelay = 0;
        int vdelay = 0;
        int ac3_id =0;
        unsigned int pid1 = 0;
        unsigned int pid2 = 0;
        char *temp_dir = NULL;
        ostringstream afile;
        ostringstream vfile;

        while (1) {
                int option_index = 0;
                static struct option long_options[] = {
                                                              {"type", required_argument, NULL, 't'
                                                              },
                                                              {"input_stream", required_argument, NULL, 'i'},
                                                              {"temp_dir", required_argument, NULL, 'd'},
                                                              {"audio_delay", required_argument, NULL, 'a'},
                                                              {"video_delay", required_argument, NULL, 'v'},
                                                              {"video_pid", required_argument, NULL, 'p'},
                                                              {"audio_pid", required_argument, NULL, 'q'},
                                                              {"ac3_id", required_argument, NULL, 'c'},
                                                              {"of",required_argument, NULL, 'o'},
                                                              {"help", no_argument , NULL, 'h'},
                                                              {0, 0, 0, 0}
                                                      };
                c = getopt_long (argc, argv, "t:o:a:v:i:hp:q:d:c:",
                                 long_options, &option_index);
                if (c == -1)
                        break;

                switch (c) {
                case 't':
                        type = optarg;
                        break;
                case 'i':
                        inpt = optarg;
                        break;
                case 'd':
                        temp_dir = optarg;
                        break;
                case 'a':
                        adelay = strtol(optarg,(char **)NULL, 0);
                        break;
                case 'p':
                        pid1 = strtol(optarg,(char **)NULL, 0);
                        break;
                case 'q':
                        pid2 = strtol(optarg,(char **)NULL, 0);
                        break;
                case 'v':
                        vdelay = strtol(optarg,(char **)NULL, 0);
                        break;
                case 'c':
                        ac3_id = strtol(optarg,(char **)NULL, 0);
                        if (ac3_id >= 0x80)
                                ac3_id-=0x80;
                        break;
                case 'o':
                        filename = optarg;
                        break;
                case 'h':
                case '?':
                default:
                        usage(argv[0]);
                }
        }

        if (optind == argc-1) {
                if ((fd_in = open(argv[optind] ,O_RDONLY| O_LARGEFILE)) < 0) {
                        cerr << "Error opening input file "
                        << argv[optind]<< endl;
                        exit(1);
                }
                cerr << "Reading from " << argv[optind] << endl;
                if (temp_dir) {
                        afile << temp_dir << "/" ;
                        vfile << temp_dir << "/" ;
                }
                afile << "tmp_as.mp2" << ends;
                vfile << "tmp_vs.m2v" << ends;
                strcpy(AudioFilename,afile.str().data());
                strcpy(VideoFilename,vfile.str().data());
        } else if (optind == argc-2 && !strcmp(inpt,"ES_STREAM")) {
                strcpy(AudioFilename,argv[optind]);
                strcpy(VideoFilename,argv[optind+1]);
        } else if (optind == argc-3 && !strcmp(inpt,"ES_STREAM")) {
                strcpy(AudioFilename,argv[optind]);
                strcpy(Audio1Filename,argv[optind+1]);
                strcpy(VideoFilename,argv[optind+2]);
        } else {
                usage(argv[0]);
        }



        if (filename)
                strcpy(ProgramFilename,filename);
        else
                strcpy(ProgramFilename,"out.mpg");
        cerr << "Output File is: " << ProgramFilename << endl;

        if (strcmp(inpt,"ES_STREAM")) {
                fda_out = open(AudioFilename,O_WRONLY|O_CREAT|O_TRUNC,
                               S_IRUSR|S_IWUSR| O_LARGEFILE);
                fdv_out = open(VideoFilename,O_WRONLY|O_CREAT|O_TRUNC,
                               S_IRUSR|S_IWUSR| O_LARGEFILE);
                if(fda_out < 0 || fdv_out < 0) {
                        cerr << "Error opening output files" << endl;
                        exit(1);
                }
        }

        if (!strcmp(type,"MPEG2"))
                SetMPEG2Mplex();
        else if (!strcmp(type,"DVD"))
                SetDVDMplex();
        else if (!strcmp(type,"SVCD"))
                SetSVCDMplex();
        else if (!strcmp(type,"MPEG1"))
                SetMPEG1Mplex();
        else if (!strcmp(type,"VCD"))
                SetVCDMplex();
        else if (!strcmp(type,"XSVCD"))
                SetXSVCDMplex();
        else if (!strcmp(type,"XVCD"))
                SetXVCDMplex();
        else
                usage(argv[0]);
        cerr << endl;


        if (!strncmp(inpt,"PS_STREAM",9)) {
                cerr << "Opening " << AudioFilename
                << " for audio demux" << endl;
                cerr << "Opening " << VideoFilename
                << " for video demux" << endl;
                ptsdiff = pes_dmx( fd_in, fda_out, fdv_out, 1+ac3_id);
        } else if (!strncmp(inpt,"TS_STREAM",9)) {
                cerr << "Opening " << AudioFilename
                << " for audio demux" << endl;
                cerr << "Opening " << VideoFilename
                << " for video demux" << endl;
                cerr << "TS demux with apid: " << pid1 << "  vpid: " << pid2;
                cerr << endl;
                ptsdiff = ts_demux(fd_in,fdv_out,fda_out,pid1,pid2,1);
        } else if (!strncmp(inpt,"TS_AC3_STREAM",13)) {
                cerr << "Opening " << AudioFilename
                << " for audio demux" << endl;
                cerr << "Opening " << VideoFilename
                << " for video demux" << endl;
                cerr << "TS_AC3 demux with apid: " << pid1 << "  vpid: " << pid2;
                cerr << endl;
                ptsdiff = ts_demux(fd_in,fdv_out,fda_out,pid1,pid2,2);
        } else if (strcmp(inpt,"ES_STREAM")) {
                usage(argv[0]);
        }


        cerr << endl;

        if ( adelay || vdelay ) {
                ptsdiff = (vdelay - adelay)*90;
        }
        cerr << "VPTS - APTS = " << ptsdiff/90 << "ms" << endl;
        if ( ptsdiff > 270000 || ptsdiff < -270000) {
                cerr << "Warning pts difference seems too large."
                << "You may have to set audio or video delay by hand" << endl;
        }
        if ( ptsdiff > 45000000 || ptsdiff < -45000000) {
                // pts difference > 10min
                // set delay to 300ms
                ptsdiff=-27000;
        }

        if (ptsdiff > 0)
                video_delay_ms += ptsdiff/90;
        else
                audio_delay_ms += -ptsdiff/90;

        if (strlen(Audio1Filename))
                domplex(1,2);
        else
                domplex(1,1);

        if (strcmp(inpt,"ES_STREAM")) {
                unlink(AudioFilename);
                unlink(VideoFilename);
        }
        fprintf(stderr,"                                                            \n");
}
