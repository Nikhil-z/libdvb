/*
 * dvbaudio - set pid and transform DVB audio
 * Copyright (C) Marcus Metzler 2002 
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

#include <getopt.h>
#include <stdlib.h>

#include <transform.h>
#include <DVB.hh>

#define SFREQ 11700000
#define LHI 10600000
#define LLO 9750000

#define IPACKS 2048


void write_es(uint8_t *buf, int count,void  *priv)
{
        ipack *p = (ipack *) priv;
        uint8_t payl = buf[8]+9+p->start-1;

        write(p->fd, buf+payl, count-payl);
        p->start = 1;
}

void handle_es(uint8_t *buf, int count,void  *priv)
{
}


void usage(char *progname)
{
        fprintf(stderr,"usage: %s [options] <pid>\n",progname);
        fprintf(stderr,"options:\n");
        fprintf(stderr,"--help,     -h         show this message\n");
        fprintf(stderr,"--adapter,  -a  <nb>   set adapter number (default 0)\n");
        fprintf(stderr,"--ipacks,   -i  <size> set internal buffer (default %d)\n", IPACKS);
        fprintf(stderr,"--demux,    -d  <dmx>  set demux (default 0)\n");
        fprintf(stderr,"--dvr,      -v  <dvr>  set dvr (default 0)\n");
        fprintf(stderr,"--frontend, -e  <fend> set frontend (default 0)\n");
        fprintf(stderr,"--freq,     -f  <freq> set frequency (default 0 = no tune)\n");
        fprintf(stderr,"--pol,      -p  <pol>  set polarisation (V or H)\n");
        fprintf(stderr,"--srate,    -s  <sr>   set signal rate\n");
        fprintf(stderr,"--fec,      -c  <fec>  set FEC (default auto)\n");
        fprintf(stderr,"--sat,      -t  <sat>  set diseqc nb. (default no diseqc)\n");
        fprintf(stderr,"\n");
        exit(1);
}




int main(int argc, char **argv)
{
        int pid=0,fd1,fd_dvr,fdf;
        char dmx[80];
        char dvr[80];
        char front[80];
        int a=0,d=0,v=0,e=0;
        char *pola;
        int c;
        int freq, pol, sr , snum, fec;
        int decode=0;
        ipack p;
        int ipacks=IPACKS;

        snum = -1;
        fec = FEC_AUTO;
        sr = 0;
        pol = 0;
        freq = 0;

        if (argc < 2) {
                usage(argv[0]);
                exit(1);
        }

        while (1) {
                int option_index = 0;
                static struct option long_options[] = {
                                                              {"adapter", required_argument, NULL, 'a'
                                                              },
                                                              {"ipacks", required_argument, NULL, 'i'},
                                                              {"demux", required_argument, NULL, 'd'},
                                                              {"front", required_argument, NULL, 'e'},
                                                              {"dvr", required_argument, NULL, 'v'},
                                                              {"freq", required_argument, NULL, 'f'},
                                                              {"pol", required_argument, NULL, 'p'},
                                                              {"srate", required_argument, NULL, 's'},
                                                              {"fec", required_argument, NULL, 'c'},
                                                              {"sat", required_argument, NULL, 't'},
                                                              {"mpg2", required_argument, NULL, 'm'},
                                                              {"help", no_argument , NULL, 'h'},
                                                              {0, 0, 0, 0}
                                                      };
                c = getopt_long (argc, argv, "ha:i:d:v:e:f:p:s:c:t:m",
                                 long_options, &option_index);

                if (c == -1)
                        break;

                switch (c) {

                case 'i':
                        ipacks=strtol(optarg,(char **)NULL,0);
                        break;

                case 'm':
                        decode=1;
                        break;

                case 'a':
                        a = strtol(optarg,(char **)NULL,0);
                        break;

                case 'd':
                        d = strtol(optarg,(char **)NULL,0);
                        break;

                case 'e':
                        e = strtol(optarg,(char **)NULL,0);
                        break;

                case 'v':
                        v = strtol(optarg,(char **)NULL,0);
                        break;

                case 'f':
                        freq = strtol(optarg,(char **)NULL,0);
                        break;
                case 'p':
                        pola = optarg;
                        switch(pola[0]) {
                        case 'V':
                        case 'v':
                                pol = 0;
                                break;

                        case 'H':
                        case 'h':
                                pol = 1;
                                break;
                        }
                        break;

                case 's':
                        sr = strtol(optarg,(char **)NULL,0);
                        break;

                case 'c':
                        fec = strtol(optarg,(char **)NULL,0);
                        break;

                case 't':
                        snum = strtol(optarg,(char **)NULL,0);
                        break;



                case 'h':
                default:
                        usage(argv[0]);
                }
        }

        if (optind) {
                pid = strtol(argv[optind],(char **)NULL,0);
        } else
                usage(argv[0]);

        if (!pid)
                usage(argv[0]);

        sprintf(dmx,"/dev/dvb/adapter%d/demux%d",a,d);
        sprintf(dvr,"/dev/dvb/adapter%d/dvr%d",a,v);
        sprintf(front,"/dev/dvb/adapter%d/frontend%d",a,v);



        if((fd1 = open(dmx,O_RDWR)) < 0) {
                perror("DEMUX DEVICE 1: ");
                return -1;
        }

        if((fd_dvr = open(dvr,O_RDONLY)) < 0) {
                perror("DVR DEVICE: ");
                return -1;
        }

        if (freq>0) {
                if((fdf = open(front,O_RDWR)) < 0) {
                        perror("FRONTEND DEVICE: ");
                        return -1;
                }

                set_sfront(fdf, freq, pol, sr , snum, (fe_code_rate_t) fec);

        }


        set_pes_filt(fd1,pid);

        if (decode)
                init_ipack(&p, ipacks, handle_es, 0);
        else
                init_ipack(&p, ipacks, write_es, 0);

        p.fd = STDOUT_FILENO;
        p.data = (void *)&p;

        ts2es_opt(fd_dvr, pid, &p, 0);

        return(0);
}
