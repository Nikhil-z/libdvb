/*
 * dvb-mpegtools for the Siemens Fujitsu DVB PCI card
 *
 * Copyright (C) 2000 Marcus Metzler for convergence integrated media GmbH
 * Copyright (C) 2002 Marcus Metzler
 * 
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

#include <ctools.h>
#define SIZE 1024

void testout(p2p *p)
{
        write(STDOUT_FILENO, p->buf, p->plength+6);
}
void write_it(u8 *buf, int count)
{
        write(STDOUT_FILENO, buf, count);
}

int main(int argc, char **argv)
{
        int fd;
        int count;
        u8 buf[SIZE];
        //trans p;
        //u8 mask[16];
        //u8 filt[16];
        //	p2a p;
        //	t2a t;
        //	a2p p;
        //	p2t_t p;
        p2p p;

        split_mpg(argv[1],strtol(argv[2],(char **)NULL, 0)*1024*1024);


        if(argc <=2) {
                if (argc == 2) {
                        fd = open(argv[1],O_RDONLY);
                        if (fd > 0)
                                fprintf(stderr,"Reading %s\n",argv[1]);
                        else
                                exit(1);
                } else {
                        fd = STDIN_FILENO;
                        fprintf(stderr,"Reading from stdin\n");
                }

                if (!strcmp(basename(argv[0]), "av_pes2anyc")) {
                        fprintf(stderr,"av_pes2anyc\n");
                        /*
                        init_trans(&p);
                        for ( i = 0; i<16; i++){
                        mask[i] = 0x00;
                        filt[i] = 0x00;
                        }
                        mask[0]= 0xFF;
                        filt[0]= 0x02;

                        set_trans_filt(&p,0,1024,mask,filt,0);
                        */
                        //	ts_av_pes2(fd);

                        //			ps_to_av_pes(fd,STDOUT_FILENO);

                        //init_p2a(&p);
                        //init_t2a(&t,-1,-1);
                        //init_a2p(&p);
                        //init_p2t(&p,NULL);
                        //init_p2p(&p, write_it, 2048);
                        count = 1;

                        //while (count > 0){
                        //count = read(fd,buf,SIZE);
                        //kpes_to_av_pes(buf, count,&p);
                        //ts_av_pes(buf, count, &t, &p);
                        //trans_filt(buf, count, &p);
                        //av_pes_to_pes(buf,count, &p);
                        //pes_to_ts(buf, count, 160, &p);
                        //get_pes(buf,count,&p,pes_repack);
                        //}
                        //ts_to_pes( fd, 80, 160);
                        //		remux2(fd,STDOUT_FILENO);
                }

        }
        return 0;
}
