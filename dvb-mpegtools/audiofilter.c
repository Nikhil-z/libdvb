/*
 * audiofilter - remove audio from dvb stream
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define PROG_STREAM_MAP  0xBC
#define PRIVATE_STREAM1  0xBD
#define PADDING_STREAM   0xBE
#define PRIVATE_STREAM2  0xBF
#define AUDIO_STREAM_S   0xC0
#define AUDIO_STREAM_E   0xDF
#define VIDEO_STREAM_S   0xE0
#define VIDEO_STREAM_E   0xEF
#define ECM_STREAM       0xF0
#define EMM_STREAM       0xF1
#define DSM_CC_STREAM    0xF2
#define ISO13522_STREAM  0xF3
#define PROG_STREAM_DIR  0xFF
#define P2P_LENGTH 2048
#define PTS_DTS_FLAGS    0xC0
#define PTS_ONLY         0x80
#define PTS_DTS          0xC0
#define MAX_PLENGTH 0xFFFF
#define MMAX_PLENGTH (8*MAX_PLENGTH)

enum{NOPES, AUDIO, VIDEO};

typedef struct p2pstruct
{
        int found;
        uint8_t buf[MMAX_PLENGTH];
        uint8_t cid;
        uint8_t subid;
        uint32_t plength;
        uint8_t plen[2];
        uint8_t flag1;
        uint8_t flag2;
        uint8_t hlength;
        uint8_t pts[5];
        int mpeg;
        uint8_t check;
        int fd1;
        int filter;
        int which;
        int done;
        void (*func)(uint8_t *buf, int count, void *p);
        int fill;
        uint8_t vobbuf[2048], *b;
}
p2p;


int filter_pes (uint8_t *buf, int count, p2p *p, int (*func)(p2p *p))
{

        int l;
        unsigned short *pl;
        int c=0;
        int ret = 1;

        uint8_t headr[3] = { 0x00, 0x00, 0x01} ;

        while (c < count && (p->mpeg == 0 ||
                             (p->mpeg == 1 && p->found < 7) ||
                             (p->mpeg == 2 && p->found < 9))
                        &&  (p->found < 5 || !p->done)) {
                switch ( p->found ) {
                case 0:
                case 1:
                        if (buf[c] == 0x00)
                                p->found++;
                        else {
                                if (p->fd1 >= 0)
                                        write(p->fd1,buf+c,1);
                                p->found = 0;
                        }
                        c++;
                        break;
                case 2:
                        if (buf[c] == 0x01)
                                p->found++;
                        else if (buf[c] == 0) {
                                p->found = 2;
                        } else {
                                if (p->fd1 >= 0)
                                        write(p->fd1,buf+c,1);
                                p->found = 0;
                        }
                        c++;
                        break;
                case 3:
                        p->cid = 0;
                        switch (buf[c]) {
                        case PROG_STREAM_MAP:
                        case PRIVATE_STREAM2:
                        case PROG_STREAM_DIR:
                        case ECM_STREAM     :
                        case EMM_STREAM     :
                        case PADDING_STREAM :
                        case DSM_CC_STREAM  :
                        case ISO13522_STREAM:
                                if (p->fd1 >= 0)
                                        write(p->fd1,buf+c,1);
                                p->done = 1;
                        case PRIVATE_STREAM1:
                        case VIDEO_STREAM_S ... VIDEO_STREAM_E:
                        case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                                p->found++;
                                p->cid = buf[c];
                                c++;
                                break;
                        default:
                                if (p->fd1 >= 0)
                                        write(p->fd1,buf+c,1);
                                p->found = 0;
                                break;
                        }
                        break;


                case 4:
                        if (count-c > 1) {
                                pl = (unsigned short *) (buf+c);
                                p->plength =  ntohs(*pl);
                                p->plen[0] = buf[c];
                                c++;
                                p->plen[1] = buf[c];
                                c++;
                                p->found+=2;
                        } else {
                                p->plen[0] = buf[c];
                                p->found++;
                                return 1;
                        }
                        break;
                case 5:
                        p->plen[1] = buf[c];
                        c++;
                        pl = (unsigned short *) p->plen;
                        p->plength = ntohs(*pl);
                        p->found++;
                        break;


                case 6:
                        if (!p->done) {
                                p->flag1 = buf[c];
                                c++;
                                p->found++;
                                if ( (p->flag1 & 0xC0) == 0x80 )
                                        p->mpeg = 2;
                                else {
                                        p->hlength = 0;
                                        p->which = 0;
                                        p->mpeg = 1;
                                        p->flag2 = 0;
                                }
                        }
                        break;

                case 7:
                        if ( !p->done && p->mpeg == 2) {
                                p->flag2 = buf[c];
                                c++;
                                p->found++;
                        }
                        break;

                case 8:
                        if ( !p->done && p->mpeg == 2) {
                                p->hlength = buf[c];
                                c++;
                                p->found++;
                        }
                        break;

                default:

                        break;
                }
        }

        if (!p->plength)
                p->plength = MMAX_PLENGTH-6;


        if ( p->done || ((p->mpeg == 2 && p->found >= 9)  ||
                         (p->mpeg == 1 && p->found >= 7)) ) {
                switch (p->cid) {

                case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                case VIDEO_STREAM_S ... VIDEO_STREAM_E:
                case PRIVATE_STREAM1:

                        memcpy(p->buf, headr, 3);
                        p->buf[3] = p->cid;
                        memcpy(p->buf+4,p->plen,2);

                        if (p->mpeg == 2 && p->found == 9) {
                                p->buf[6] = p->flag1;
                                p->buf[7] = p->flag2;
                                p->buf[8] = p->hlength;
                        }

                        if (p->mpeg == 1 && p->found == 7) {
                                p->buf[6] = p->flag1;
                        }


                        if (p->mpeg == 2 && (p->flag2 & PTS_ONLY) &&
                                        p->found < 14) {
                                while (c < count && p->found < 14) {
                                        p->pts[p->found-9] = buf[c];
                                        p->buf[p->found] = buf[c];
                                        c++;
                                        p->found++;
                                }
                                if (c == count)
                                        return 1;
                        }

                        if (p->mpeg == 1 && p->which < 2000) {

                                if (p->found == 7) {
                                        p->check = p->flag1;
                                        p->hlength = 1;
                                }

                                while (!p->which && c < count &&
                                                p->check == 0xFF) {
                                        p->check = buf[c];
                                        p->buf[p->found] = buf[c];
                                        c++;
                                        p->found++;
                                        p->hlength++;
                                }

                                if ( c == count)
                                        return 1;

                                if ( (p->check & 0xC0) == 0x40 && !p->which) {
                                        p->check = buf[c];
                                        p->buf[p->found] = buf[c];
                                        c++;
                                        p->found++;
                                        p->hlength++;

                                        p->which = 1;
                                        if ( c == count)
                                                return 1;
                                        p->check = buf[c];
                                        p->buf[p->found] = buf[c];
                                        c++;
                                        p->found++;
                                        p->hlength++;
                                        p->which = 2;
                                        if ( c == count)
                                                return 1;
                                }

                                if (p->which == 1) {
                                        p->check = buf[c];
                                        p->buf[p->found] = buf[c];
                                        c++;
                                        p->found++;
                                        p->hlength++;
                                        p->which = 2;
                                        if ( c == count)
                                                return 1;
                                }

                                if ( (p->check & 0x30) && p->check != 0xFF) {
                                        p->flag2 = (p->check & 0xF0) << 2;
                                        p->pts[0] = p->check;
                                        p->which = 3;
                                }

                                if ( c == count)
                                        return 1;
                                if (p->which > 2) {
                                        if ((p->flag2 & PTS_DTS_FLAGS)
                                                        == PTS_ONLY) {
                                                while (c < count &&
                                                                p->which < 7) {
                                                        p->pts[p->which-2] =
                                                                buf[c];
                                                        p->buf[p->found] =
                                                                buf[c];
                                                        c++;
                                                        p->found++;
                                                        p->which++;
                                                        p->hlength++;
                                                }
                                                if ( c == count)
                                                        return 1;
                                        } else if ((p->flag2 & PTS_DTS_FLAGS)
                                                        == PTS_DTS) {
                                                while (c < count &&
                                                                p->which< 12) {
                                                        if (p->which< 7)
                                                                p->pts[p->which
                                                                       -2] =
                                                                               buf[c];
                                                        p->buf[p->found] =
                                                                buf[c];
                                                        c++;
                                                        p->found++;
                                                        p->which++;
                                                        p->hlength++;
                                                }
                                                if ( c == count)
                                                        return 1;
                                        }
                                        p->which = 2000;
                                }

                        }

                        while (c < count && p->found < p->plength+6) {
                                l = count -c;
                                if (l+p->found > p->plength+6)
                                        l = p->plength+6-p->found;
                                memcpy(p->buf+p->found, buf+c, l);
                                p->found += l;
                                c += l;
                        }
                        if(p->found == p->plength+6) {
                                if (func(p)) {
                                        if (p->fd1 >= 0) {
                                                write(p->fd1,p->buf,
                                                      p->plength+6);
                                        }
                                } else
                                        ret = 0;
                        }
                        break;
                }


                if ( p->done ) {
                        if( p->found + count - c < p->plength+6) {
                                p->found += count-c;
                                c = count;
                        } else {
                                c += p->plength+6 - p->found;
                                p->found = p->plength+6;
                        }
                }

                if (p->plength && p->found == p->plength+6) {
                        p->found = 0;
                        p->done = 0;
                        p->plength = 0;
                        memset(p->buf, 0, MAX_PLENGTH);
                        if (c < count)
                                return filter_pes(buf+c, count-c, p, func);
                }
        }
        return ret;
}

int audio_pes_filt(p2p *p)
{
        uint8_t off;

        switch(p->cid) {
        case PRIVATE_STREAM1:
                if ( p->cid == p->filter) {
                        off = 9+p->buf[8];
                        if (p->buf[off] == p->subid) {
                                return 1;
                        }
                }
                break;

        case AUDIO_STREAM_S ... AUDIO_STREAM_E:
                if ( p->cid == p->filter)
                        return 1;
                break;

        default:
                return 1;
                break;
        }
        return 0;
}

void buf_audio_filter(int fdout, uint8_t *dat, int count, p2p *p)
{
        int rest = 0;

        if (count > 0) {
                if ( p->fill+count <= 2048) {
                        memcpy(p->b, dat, count);
                        p->fill+=count;
                        p->b += count;
                        if(p->fill == 2048) {
                                if (filter_pes(p->vobbuf,2048,
                                                p,audio_pes_filt))
                                        write(fdout,p->vobbuf,2048);
                                p->b = p->vobbuf;
                                p->fill = 0;
                        }
                } else {
                        rest = 2048 - p->fill;
                        memcpy(p->b , dat, rest );
                        if (filter_pes(p->vobbuf,2048,
                                        p,audio_pes_filt))
                                write(fdout,p->vobbuf,2048);
                        p->b = p->vobbuf;
                        p->fill = 0;
                        buf_audio_filter(fdout, dat+rest, count-rest, p);
                }
        }
}

void filter_audio_from_pes(int fdin, int fdout, uint8_t id, uint8_t subid)
{
        p2p p;
        int count = 1;
        uint8_t buf[2048];

        p.found = 0;
        p.cid = 0;
        p.mpeg = 0;
        memset(p.buf,0,MMAX_PLENGTH);
        memset(p.vobbuf,0,2048);
        p.done = 0;
        p.fd1 = -1;
        p.filter = id;
        p.subid = subid;
        p.fill = 0;
        p.b = p.vobbuf;

        while (count > 0) {
                count = read(fdin,buf,100);
                buf_audio_filter(fdout, buf, count, &p);
        }
}



void usage(char *progname)
{
        fprintf(stderr,"usage: %s --pid <pid> [options]\n",progname);
        fprintf(stderr,"options:\n");
        fprintf(stderr,"--help,     -h              show this message\n");
        fprintf(stderr,"--infile,   -i  <filename>  input (default=stdin)\n");
        fprintf(stderr,"--outfile,  -o  <filename>  output (default=stdout)\n");
        fprintf(stderr,"--id,       -a  <id>        audio id (default=0xc0)\n");
        fprintf(stderr,"--subid,    -s  <subid>     subid for privat stream (e.g. -a 0xbd -s 0x80 for first AC3)\n");
        fprintf(stderr,"\n");
        exit(1);
}




int main(int argc, char **argv)
{
        int fdin=STDIN_FILENO,fdout=STDOUT_FILENO;
        uint8_t id=0xc0, subid=0;
        int c;

        while (1) {
                int option_index = 0;
                static struct option long_options[] = {
                                                              {"infile", required_argument, NULL, 'i'
                                                              },
                                                              {"outfile", required_argument, NULL, 'o'},
                                                              {"id", required_argument, NULL, 'a'},
                                                              {"subid", required_argument, NULL, 's'},
                                                              {"help", no_argument , NULL, 'h'},
                                                              {0, 0, 0, 0}
                                                      };
                c = getopt_long (argc, argv, "hi:o:a:s:",
                                 long_options, &option_index);

                if (c == -1)
                        break;

                switch (c) {

                case 'i':
                        if ((fdin = open(optarg,O_RDONLY|O_LARGEFILE)) <0) {
                                perror(optarg);
                                exit(1);
                        }
                        break;

                case 'o':
                        if ((fdout = open(optarg,O_WRONLY|O_CREAT|O_TRUNC
                                          |O_LARGEFILE,
                                          S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|
                                          S_IROTH|S_IWOTH)) <0) {
                                perror(optarg);
                                exit(1);
                        }
                        break;

                case 'a':
                        id = strtol(optarg,(char **)NULL,0);
                        break;

                case 's':
                        subid = strtol(optarg,(char **)NULL,0);
                        break;

                case 'h':
                default:
                        usage(argv[0]);
                }
        }

        filter_audio_from_pes(fdin, fdout, id, subid);
        return(0);
}
