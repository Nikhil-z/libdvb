#include <iostream>
#include "DVB.hh"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <netdb.h>
#include <sys/times.h>


#include <ctools.h>
#define FILELEN 80

int handler_fd;


int found_ecm (Channel *chan)
{
	ecm_t *ecm = &chan->ecm;

	cerr << "found ecm" << endl;
	for (int i=0; i < ecm->num; i++){
		cerr << "   ECM: sysid 0x" << hex << ecm->sysid[i] << "  pid 0x" << ecm->pid[i];
		cerr << "  dlength " << ecm->length[i] << "  descriptor:";
		if (ecm->length[i]){
			for (int j=0; j< ecm->length[i]; j++){
				cerr << " 0x" << int(ecm->data[j+i*MAX_ECM_DESC]);
			}
		}
		cerr << endl;
	}
	return 1;
}

void write_out(uint8_t *buf, int count,void  *p)
{
	if ((buf[3] & 0xe0) == 0xe0) buf[3] = 0xe0;
        write(STDOUT_FILENO, buf, count);
}

void help(){
	cerr << "usage: cam_test [options]" << endl
	     << "-h          print this help" << endl
	     << "-s          activate string input/search for channel" << endl
	     << "-f <name>   open name as dvbrc file" << endl
	     << "-a <N>      use /dev/dvb/adaptorN" << endl
	     << "-d          activate dvr output" << endl
	     << "-o          output dvr to stdout" << endl
	     << "-p          output dvr as PS" << endl
	     << "-u <name>   use socket name for io with cam_set" << endl
	     << "-c <N>      tune to channel N at startup" << endl
	     << "-v <name>   use name as VTX directory" << endl
	     << "-n          don't use cam" << endl
	     << "-q <freq>   frequency in Hz" << endl
	     << "-Q          ignore stdin input, allow closing stdin" << endl
	     << "-l <pol>    polarity (V/H)" << endl
	     << "-r <pol>    sample rate in Hz" << endl
	     << "-i <sid>    service pid"<< endl
	     << "-t <apid>   audio pid"<< endl
	     << "-y <inv>    inversion (0=OFF(DEFAULT),1=ON,2=AUTO)"
	     << "-x <dis>    diseqc"<< endl
	     << "-z          show switch time" << endl;
}

#define IN_SIZE TS_SIZE*10
#define IPACKS 2048
	
int main(int argc, char **argv)
{
	uint8_t buf[IN_SIZE];
        uint8_t mbuf[TS_SIZE];
	int cnum=-1;
	int c,count ;
	int show_time=0;
	clock_t clock1, clock2;
	struct tms ts1,ts2;
	Channel *chan;
	char filename[FILELEN];
	char *sockname=0;
	char *vtxdir=0;
	char *pol=0;
	int adapter=0;
	int dvr=0;
	int np=1;
	uint8_t str[12];
	int fd_dvr=-1;
	char devname[80];        
	ipack pa, pv;
        ipack *p;
	uint16_t apid=NOPID;
	uint16_t vpid=NOPID;
	uint16_t ttpid=NOPID;
	int i=0;
	int cam=1;
	int ts2ps=0;		
	int strsearch=0;
	int freq=0;
	int srate=0;
	int sid=NOPID;
	int napid=NOPID;
	int nvpid=NOPID;
	int dis=0;
	int inv =0;
	bool no_stdin=false;

	memset(filename, 0, FILELEN);

        for (;;) {
                if (-1 == (c = getopt(argc, argv, 
				      "hf:a:du:onc:pv:V:sq:Ql:r:i:t:x:y:z")))
                        break;
                switch (c) {
		case 'h':
			help();
			exit(0);
			break;
		case 's':
			strsearch=1;
			break;
		case 'f':
			strncpy(filename, optarg, FILELEN);	
                        break;
		case 'd':
			dvr = 1;
			break;
		case 'a':
			adapter = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'u':
			sockname = strdup(optarg);
			break;
		case 'o':
			dvr = 2;
			break;
		case 'n':
			cam = 0;
			break;
		case 'c':
			cnum = strtol(optarg,(char **)NULL,0);
			break;
		case 'p':
			ts2ps = 1;
			break;
		case 'v':
			vtxdir = strdup(optarg);
			break;
		case 'q':
			freq = strtol(optarg,(char **)NULL,0);
			if(!freq)
				freq = -1;
			break;
		case 'Q':
			no_stdin=true;
			break;
		case 'l':
			pol = strdup(optarg);
			break;
		case 'r':
			srate = strtol(optarg,(char **)NULL,0);
			break;
		case 'i':
			sid =  strtol(optarg,(char **)NULL,0);
			break;
		case 't':
			napid = strtol(optarg,(char **)NULL,0);
			break;
		case 'V':
			nvpid = strtol(optarg,(char **)NULL,0);
			break;
		case 'x':
			dis = strtol(optarg,(char **)NULL,0);
			break;
		case 'y':
			inv = strtol(optarg,(char **)NULL,0);
			break;
		case 'z':
			show_time=1;
			break;
		}
	}

	DVB dvb(adapter);
	dvb.set_showtime(show_time);
	if(vtxdir) {
		dvb.set_vtxdir(vtxdir);
		free(vtxdir);
	}
	sprintf(devname,DVR_DEV,adapter,0);

	if (!freq){
		if (get_dvbrc(filename,dvb,0,FILELEN)) {
			cerr << "Found DVB input file " << filename << endl;
		} else {
			cerr << "Could not find DVB input file " 
			     << filename << endl;
			exit(1);
		}
	} else {
		Transponder tp;
		Channel nchan;
		dvb.AddLNB(1, 1, 9750000, 10600000, 11700000, dis, NOID, NOID);
		dvb.AddSat( 1, 1,"who cares", 10700000 , 12700000);
		if (inv==1) tp.inversion = INVERSION_ON;
		else if (inv==2) tp.inversion = INVERSION_AUTO;
		else if (inv==0) tp.inversion = INVERSION_OFF;
		tp.satid = 1; 
		tp.freq = freq;
		tp.srate = srate;
		tp.type = FE_QPSK;
		tp.fec = FEC_AUTO;
		if (pol && pol[0] == 'H') tp.pol = 1;
		else if (pol && pol[0] == 'V') tp.pol = 0;
		nchan.tpid = dvb.tps[dvb.AddTP(tp)].id;
		nchan.satid = dvb.tps[dvb.AddTP(tp)].satid;
		nchan.pnr = sid;
		if (napid!=NOPID){
			nchan.apids[0] = napid;
			nchan.apidnum = 1;
		}
		if (nvpid!=NOPID)
			nchan.vpid = nvpid;

		cnum = dvb.AddChannel(nchan);
	}
	
	if (dvr){
		dvb.enable_DVR_other();			
		if (dvr>1) np = 2;
	}
	struct pollfd pfd[np];

	pfd[0].fd = STDIN_FILENO;
        pfd[0].events = POLLIN;
	if(dvr>1){
		pfd[1].fd = -1;
		pfd[1].events = 0;
	}

	while(1){
		if (cnum >=0 && cnum < dvb.num[CHAN]){
			if (dvr>1) close(fd_dvr);
			if (show_time) clock1=times(&ts1);
			if (dvb.SetChannel(cnum,NULL,&apid,&vpid,freq>=0)<0){
				cerr << "Tuning failed for " 
				     << dvb.chans[cnum].name
				     << endl;
				cnum = -1;
				continue;
			}
			if (show_time){
				clock2=times(&ts2);
				cerr << "Switch time = " 
				     <<	(double)(clock2-clock1)
					/double(sysconf(_SC_CLK_TCK)) 
				     << "s" << endl;
			}
			if (dvb.chans[cnum].vpid == NOPID){
				cerr << "MISSING VIDEO PID" << endl;
			}
			
			if (dvb.chans[cnum].apidnum ==0){
				cerr << "MISSING AUDIO PID" << endl;
			}
			
			ttpid = dvb.chans[cnum].ttpid;
			cerr << "Channel: " << dvb.chans[cnum].name
			     << " apid: 0x" << hex << apid 
			     << " vpid: 0x" << vpid 
			     << " ttpid: 0x" << ttpid<< endl;
			
			chan = &dvb.chans[cnum];
			chan->ecm_callback= found_ecm;
			if (cam && !chan->ecm.num) dvb.check_ecm(chan);
			if (cam && chan->ecm.num){
				if(sockname)
					handler_fd = udp_client_connect(sockname);
				else
					handler_fd = tcp_client_connect("localhost", 4711);
				ecm_t *ecm = &chan->ecm;
				for (int i=0; i < ecm->num; i++){
					
					memcpy(str,&chan->pnr,2);
					memcpy(str+2,&chan->vpid,2);
					memcpy(str+4,&chan->apids[0],2);
					memcpy(str+6,&chan->apids[1],2);
					memcpy(str+8,&chan->ac3pid,2);
					memcpy(str+10,&ecm->length[i],2);
					cerr << "CAM Test: 0x" << chan->pnr << " 0x"
					     << chan->vpid << " 0x"
					     << chan->apids[0] << " 0x"
					     << chan->apids[1] << " 0x"
					     << chan->ac3pid << " 0x"
					     << ecm->length[i] << endl;
					
					client_send_msg(handler_fd, str, 12);
					client_send_msg(handler_fd, 
							ecm->data+(i*MAX_ECM_DESC), 
							ecm->length[i]);
				}
				
				memset(str+2,0,10);
				client_send_msg(handler_fd, str, 12);
				memset(str, 0, 12);
				client_send_msg(handler_fd, str, 12);
			}

			
			if (dvr>1){
				fd_dvr = open(devname, O_RDONLY);
				if (fd_dvr < 0) {
					cerr << "Could not open " 
					     << devname << endl;
					perror(devname);
				} else {
					pfd[1].fd = fd_dvr;
					pfd[1].events = POLLIN;
					
					if(ts2ps) {
						init_ipack(&pa, IPACKS,write_out, 1);
						init_ipack(&pv, IPACKS,write_out, 1);
					}
					
					if ((count = 
					     save_read(fd_dvr,mbuf,TS_SIZE))<0)
						perror("reading");
					
					for ( i = 0; i < 188 ; i++){
						if ( mbuf[i] == 0x47 ) break;
					}
					
					if ( i == 188){
						fprintf(stderr,"Not a TS\n");
					} else {
						memcpy(buf,mbuf+i,TS_SIZE-i);
						if ((count = 
						     save_read(fd_dvr,mbuf,i))<0)
							perror("reading");
						memcpy(buf+TS_SIZE-i,mbuf,i);
						i = 188;
					}
					
				}
			}
			cnum = -1;
		}
		
		if (poll(pfd,np,1000)>0){

			if (dvr>1 && (pfd[1].revents & POLLIN)){
				if ((count = save_read(fd_dvr,buf+i,IN_SIZE-i)+i)<0)
					perror("reading");
				
				for(i = 0; i < count; i+= TS_SIZE){
					uint8_t off = 0;
					
					if ( count - i < TS_SIZE) break;
					
					uint16_t pid = get_pid(buf+i+1);
					if (!(buf[3+i]&0x10)) // no payload?
						continue;
					
					if ( buf[1+i]&0x80){
						fprintf(stderr,"Error in TS for PID: %d\n",
							pid);
					}
					
					if (ts2ps && pid == vpid){
						p = &pv;
					} else if (ts2ps && pid == apid){
						p = &pa;
					} else if (pid == ttpid){
						int l, j=0;
						uint8_t mpag, mag, pack;
						for(l=0; l<4; l++) {
							if(buf[4+l*46+i]==2) {
								for(j=(8+l*46)+i; 
								    j<(50+l*46)+i; j++)
									buf[j] = invtab[
										buf[j]];
								mpag=deham(buf[0x8+l*46+i],
									   buf[0x9+l*46+i]);
								mag=mpag&7;
								pack=(mpag>>3)&0x1f;
								dvb.add_vtx_line(
									&dvb.magazin[mag],
									pack,
									&buf[10+l*46+i],
									pid);
							}
						}
						continue;
					} else if(ts2ps) continue;

					if ( ts2ps && buf[1+i]&0x40) {
						if (p->plength == MMAX_PLENGTH-6){
							p->plength = p->found-6;
							p->found = 0;
							send_ipack(p);
							reset_ipack(p);
						}
					}
					

					if ( ts2ps && buf[3+i] & 0x20) {  // adaptation field?
						off = buf[4+i] + 1;
					}

					if(ts2ps)
						instant_repack(buf+4+off+i, TS_SIZE-4-off, p);
					else
						write(1, buf+i, TS_SIZE);
				}
				i = 0;
			}

			if (!no_stdin && (pfd[0].revents & POLLIN)){
				if (strsearch){
					char cdummy[MAXNAM+1];
					memset(cdummy,0,MAXNAM+1);
					getname(cdummy,cin,char(0),'\n');
					int nn=0;
					if (!strncmp(cdummy,"exit",4)) exit(0);
					else {
						while ( nn < dvb.num[CHAN] &&
							strncmp(cdummy, dvb.chans[nn].name, 
								strlen(cdummy))){
							nn++;
						}
						cnum =nn;
						if (cnum == dvb.num[CHAN])
							cerr << "channel not found" << endl;
					}
				} else {
					cin >> cnum;
					if (cnum < 0){
						exit(0);
					}
				}
			}
		}
	}
}
