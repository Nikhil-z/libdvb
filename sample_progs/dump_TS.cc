#include <iostream>
#include "DVB.hh"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <netdb.h>

#include <ctools.h>
#define FILELEN 80

int handler_fd;

void help(){
	cerr << "-h          print this help" << endl
	     << "-a <N>      use /dev/dvb/adaptorN" << endl
	     << "-c <N>      tune to channel N at startup" << endl
	     << "-f <name>   open name as dvbrc file" << endl
	     << "-l <pol>    polarity (V/H)" << endl
	     << "-o          only PIDS" << endl
 	     << "-p <fpid>   filter fpid" << endl
	     << "-q <freq>   frequency in Hz" << endl
	     << "-r <srate>  sample rate in Hz" << endl
	     << "-s          activate string input/search for channel" << endl
	     << "-x <dis>    diseqc"<< endl;
}

#define IN_SIZE TS_SIZE*10
#define IPACKS 2048
	
int main(int argc, char **argv)
{
	uint8_t buf[IN_SIZE];
        uint8_t mbuf[TS_SIZE];
	int cnum=-1;
	int c,count ;
	Channel *chan;
	char filename[FILELEN];
	char *pol=0;
	int adapter=0;
	int fd_dvr=-1;
	char devname[80];        
	int i=0;
	int strsearch=0;
	int freq=0;
	int srate=0;
	int dis=0;
	int fdd = -1;
	uint16_t fpid=NOPID;
	int pidonly = 0;
	int foundpids = 0;
	#define MAXP 1024
	uint16_t fpids[MAXP];

	memset(filename, 0, FILELEN);

        for (;;) {
                if (-1 == (c = getopt(argc, argv, "ha:c:f:l:op:q:r:sx:")))
                        break;
                switch (c) {
		case 'a':
			adapter = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'c':
			cnum = strtol(optarg,(char **)NULL,0);
			break;
		case 'f':
			strncpy(filename, optarg, FILELEN);	
                        break;
		case 'l':
			pol = strdup(optarg);
			break;
		case 'o':
			pidonly = 1;
			break;
		case 'p':
			fpid = strtol(optarg,(char **)NULL,0);
			break;
		case 'q':
			freq = strtol(optarg,(char **)NULL,0);
			if(!freq)
				freq = -1;
			break;
		case 'r':
			srate = strtol(optarg,(char **)NULL,0);
			break;
		case 's':
			strsearch=1;
			break;
		case 'x':
			dis = strtol(optarg,(char **)NULL,0);
			break;
		default:
		case 'h':
			help();
			exit(0);
			break;
		}
	}

	DVB dvb(adapter);
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
		tp.satid = 1; 
		tp.freq = freq;
		tp.srate = srate;
		tp.type = FE_QPSK;
		tp.fec = FEC_AUTO;
		if (pol && pol[0] == 'H') tp.pol = 1;
		else if (pol && pol[0] == 'V') tp.pol = 0;
		nchan.tpid = dvb.tps[dvb.AddTP(tp)].id;
		nchan.satid = dvb.tps[dvb.AddTP(tp)].satid;
		nchan.pnr = 0;
		cnum = dvb.AddChannel(nchan);
	}
	
	struct pollfd pfd[2];

	pfd[0].fd = STDIN_FILENO;
        pfd[0].events = POLLIN;
	pfd[1].fd = -1;
	pfd[1].events = 0;
	while(1){
		if (cnum >=0 && cnum < dvb.num[CHAN]){
			close(fd_dvr);
			if (fdd >=0) close(fdd);
			chan = &dvb.chans[cnum];
			dvb.get_front();
			fdd = dvb.SetTP(chan->tpid, chan->satid);
			if ( fdd < 0 || dvb.set_front() <0){
				cerr << "Tuning failed for " 
				     << chan->name << " transponder"
				     << endl;
				cnum = -1;
				continue;
			}
			cerr << "Tuning transponder of channel "
			     << chan->name << endl;
			if ((fdd = dvb.SetFullFilter(fpid))<0)
				cerr << "Couldn't set full filter" << endl; 
			

			fd_dvr = open(devname, O_RDONLY);
			cerr << "Opening DVR device" 
			     << devname << endl; 
			if (fd_dvr < 0) {
				cerr << "Could not open " 
				     << devname << endl;
				perror(devname);
			} else {
				pfd[1].fd = fd_dvr;
				pfd[1].events = POLLIN;
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
			cnum = -1;
			if (pidonly) cerr << "PIDS: ";
		}
		
		if (poll(pfd,2,1000)>0){
			if ((pfd[1].revents & POLLIN)){
				if ((count = save_read(fd_dvr,buf+i,IN_SIZE-i)+i)<0)
					perror("reading");
				
				if (pidonly){
					for(i = 0; i < count; i+= TS_SIZE){
						if ( count - i < TS_SIZE) break;
						uint16_t pid = get_pid(buf+i+1);
						int found = 0;
						for (int j=0; j < foundpids && j < MAXP; j++){
							if (pid == fpids[j]){
								found = 1;
								break;
							}
						}
						if (!found && foundpids < MAXP){
							cerr << "0x" 
							     << HEX(4) << pid << "  ";
							fpids[foundpids] = pid;
							foundpids++;
						}
						if (!found && !(foundpids%16)) cerr << endl << "PIDS: ";
						if (!found && foundpids >= MAXP)
							cerr << "no more PID space left" << endl;
					} 
					
				} else write(STDOUT_FILENO, buf+i, IN_SIZE-i);
				i = 0;
			}

			if (pfd[0].revents & POLLIN){
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
