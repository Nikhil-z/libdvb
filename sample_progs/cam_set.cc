#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;
#include <fstream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/dvb/ca.h>
#include <sys/ioctl.h>
#include <signal.h>
#define TIMEOUT 30

#include "ci.hh"
#include "cam_menu.hh"

#define CA_DEV    "/dev/dvb/adapter%d/ca%d"
#define DEMUX_DEV "/dev/dvb/adapter%d/demux%d"

#define MENUTIMEOUT 10

enum{SEARCH, NONE,CAM};


#define SERV_SOCK 4711

#define NOPID 0xffff

void show_buf(uint8_t *buf, int length)
{
	printf("\n");
	for (int i=0; i<length; i+=8){
		int j;
		for (j=0; j < 8 && j+i<length; j++)
			printf("0x%02x ", int(buf[i+j]));
		for (int r=j; r<8; r++) 			
			printf("     ");

		for (j=0; j < 8 && j+i<length; j++){
			switch(buf[i+j]){
			case '0'...'Z':
			case 'a'...'z':
				printf("%c", buf[i+j]);
				break;
			default:
				printf(".");
			}
		}
		printf("\n");
	}
}

int my_read(int fd, uint8_t *buf, int length)
{
	int count = 0;
	while(count < length){
		int re;
		if ((re=read(fd, (char *)(buf+count), length-count))>=0) count+=re;
		else return re;
	}
	return count;
} 

int child_events = 0;

void child_sig_handler (int x) {
	child_events++;
	cerr << "Signal Handler" << endl;
	signal (SIGCHLD, child_sig_handler);
}



enum {MENU_START, MENU_ENQUIRY, MENU_NONE};

#define BLEN 2048
int main(int argc, char **argv)
{
        int c;
	time_t startt=0;
	time_t mtime=0;
	uint16_t length=0;
	ifstream con(argv[1]);	
	cCiHandler *ciHandler;
	struct timeval tv;
	fd_set rfds;
	char devname[80];
	int adapter = 0;
	int minor = 0;
	int state = SEARCH;
	uint8_t buffer[BLEN];
	uint16_t vpid=0;
	uint16_t apid1=0;
	uint16_t apid2=0;
	uint16_t dpid1=0;
	uint16_t sid=0;
	uint16_t set_sid=0;
	uint16_t current_vpid=0;
	uint16_t current_apid1=0;
	uint16_t current_apid2=0;
	uint16_t current_dpid1=0;
	uint16_t current_sid=0;
	sockaddr_in ip_name1;
	sockaddr_un udp1;
	sockaddr_in ip_name2;
	sockaddr_un udp2;
	char *sockname1=0;
	char *sockname2=0;
	int ip_sock1=-1;
	int ip_sock2=-1;
	int conn1=-1;
	int conn2=-1;
        bool localonly=false;
	int timeout=10000;
	int menustate=MENU_NONE;
	cCiMenu *ciMenu = NULL;
	cCiEnquiry *ciEnquiry = NULL;
	uint8_t mrange = 0;
	int EnLength = 0;
	char *input = NULL;
	int menu_sock = MENU_SOCK;
	int simple_ci = 0;
	ca_slot_info_t sinfo;


	for (;;) {
                if (-1 == (c = getopt(argc, argv, "a:m:lu:v:s:")))
                        break;
                switch (c) {
		case 'a':
			adapter = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'm':
			minor = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'l':
                        localonly=true;
                        break;
		case 'u':
			sockname1=strdup(optarg);
			unlink(sockname1);
			break;
		case 'v':
			sockname2=strdup(optarg);
			unlink(sockname2);
			break;
		case 's':
			menu_sock=strtol(optarg,(char **) NULL, 0);
			break;
		}
	}

	sprintf(devname,CA_DEV,adapter,minor);
	int cafd;
	if ((cafd = open(devname,O_RDONLY))<0){
		perror("ca device");
		exit(1);
	}
	ca_caps_t caps;
	ioctl(cafd, CA_GET_CAP, &caps);
	if (caps.slot_num < 1){
		cerr << "No CI Slots found" << endl;
		exit(1);
	}
	int cams=0;
	for (uint32_t i=0; i<caps.slot_num; i++){
	
		sinfo.num = i;
		if (ioctl(cafd, CA_GET_SLOT_INFO, &sinfo) <0){
			perror("slot info");
			exit(1);
		}
		if (sinfo.flags & CA_CI_MODULE_PRESENT) cams++;
	}

	if (sinfo.type == CA_CI ){
		cerr << "CI interface is high level only" << endl;
		simple_ci = 1;
		ci_reset(cafd,0);
	}

	if (!simple_ci && !cams){
		cerr << "No CAMs present" << endl;
		exit(1);
	}
	if (!simple_ci){
		cerr << "Found " << cams << "CAM(s)" << endl;
		close(cafd);
	}

	if (!simple_ci){
		ciHandler = cCiHandler::CreateCiHandler(devname);
		if(!ciHandler) {
			cout << "No DVB card found" << endl;
			exit(0);
		}
	}

	cout << "Using /dev/dvb/adapter" << adapter << "/ca" << minor << endl;
	

	int numslot= sinfo.num;
	if (simple_ci) numslot=1;
	cout << "Found "<< numslot << " slots" << endl;

	uint8_t cam_buf[BLEN*numslot];
	int cam_length[numslot];
	int enread = 0;

	if(sockname1) {
		ip_sock1 = udp_listen(&udp1, sockname1);
	} else {
		if(localonly)
			ip_sock1 = tcp_listen(&ip_name1, SERV_SOCK, 0x0100007f);
		else
			ip_sock1 = tcp_listen(&ip_name1, SERV_SOCK);
	}
	
        if (ip_sock1 < 0){
                perror("listen socket");
                exit(1);
        }

	if(sockname2) {
		ip_sock2 = udp_listen(&udp2, sockname2);
	} else {
		if(localonly)
			ip_sock2 = tcp_listen(&ip_name2, menu_sock, 0x0100007f);
		else
			ip_sock2 = tcp_listen(&ip_name2, menu_sock);
	}
	
        if (ip_sock2 < 0){
                perror("listen socket");
                exit(1);
        }


	startt = time(0);
	mtime = time(0)+MENUTIMEOUT;

	
	sigset_t sigmask, orig_sigmask;
	sigemptyset (&sigmask);
	sigaddset (&sigmask, SIGCHLD);
	sigprocmask (SIG_BLOCK, &sigmask, &orig_sigmask);
	
	signal (SIGCHLD, child_sig_handler);

	while(1){
		tv.tv_sec = 0;
		tv.tv_usec = timeout;
		FD_ZERO(&rfds);

		FD_SET(ip_sock1,&rfds);
		FD_SET(ip_sock2,&rfds);
		int check = (ip_sock1 > ip_sock2) ? ip_sock1 : ip_sock2;

		if (conn1 >= 0){
			FD_SET(conn1,&rfds);
			if (conn1 > check) check =conn1;
		}

		if (conn2 >= 0){
			FD_SET(conn2,&rfds);
			if (conn2 > check) check =conn2;
		}
		
		int ret = select(check+1, &rfds, NULL, NULL, &tv);

		if (ret){

			if (FD_ISSET(ip_sock1, &rfds) && conn1 < 0 ){
		  
				while( (conn1= (sockname1 ? accept_udp(ip_sock1,
								       (struct sockaddr_un *)
								       &ip_name1)
						: accept_tcp(ip_sock1,
							     (struct sockaddr_in *)
							     &ip_name1)
						)) < 0) cerr << "Connection attempt" << endl;
				
				cerr << "CI HANDLER: connection established" 
				     << endl;
				continue;
			}

			if (FD_ISSET(ip_sock2, &rfds) && conn2 <0){
				while( (conn2= (sockname2 ? accept_udp(ip_sock2,
								       (struct sockaddr_un *)
								       &ip_name2)
						: accept_tcp(ip_sock2,
							     (struct sockaddr_in *)
							     &ip_name2)
						)) < 0) cerr << "Connection attempt" << endl;

				cerr << "CAM MENU: connection established" 
				     << endl;
				continue;
			}

			if (conn1>=0 && FD_ISSET(conn1, &rfds)){
				timeout = 100;

				my_read(conn1,buffer,12);
				memcpy(&sid,buffer,2); 
				memcpy(&vpid,buffer+2,2);
				memcpy(&apid1,buffer+4,2);
				memcpy(&apid2,buffer+6,2);
				memcpy(&dpid1,buffer+8,2);
				memcpy(&length,buffer+10,2);
				
				if (sid && !vpid && !apid1 && !apid2 && !dpid1 
				    && !length){
					state = CAM;
					cerr << "state: CAM" << endl;
					continue;
				}


				if (!sid && !vpid && !apid1 && !apid2 && !dpid1 
				    && !length){
					close(conn1);
					conn1=-1;

					cerr << "CI HANDLER: client hung up" 
					     << endl;
					//state = SEARCH;
					continue;
				}

				if (sid==1 && vpid==1 && apid1==1 && apid2==1 && dpid1==1 
				    && length==1){
					cerr << "CI HANDLER: ending handler" 
					     << endl;
					exit(0);
				}

				current_sid = sid;
				current_vpid = vpid;
				current_apid1 = apid1;
				current_apid2 = apid2;
				current_dpid1 = dpid1;
				if (current_vpid == NOPID){
					cerr << "MISSING VIDEO PID" << endl;
				}

				if (current_apid1 == NOPID){
					cerr << "MISSING AUDIO PID" << endl;
				}


				if (length+12 > 2048){
					cerr << "section too long" << endl;
					exit(1);
				}
				my_read(conn1,buffer+12,length);

				cerr << hex << "CI HANDLER: 0x" << current_sid << " 0x" 
				     << current_vpid << " 0x" 
				     << current_apid1 << " 0x" 
				     << current_apid2 << " 0x" 
				     << current_dpid1 
				     << " 0x" << length << " |";
				for (int i=12 ; i<length+12; i++){
					cerr << " 0x" << int(buffer[i]);
				}
				cerr << " ok" << endl;

				uint16_t sysid =(buffer[14]<<8)|buffer[15];
				cerr << "SysID " << sysid << endl;

				for (int Slot = 0; Slot < numslot;Slot++) {
					int done = 0;
					int count = 0;
					const unsigned short *caids;

					cerr << "Slot " << Slot << endl;
					if (!simple_ci){
						caids = ciHandler->GetCaSystemIds(Slot);

						if (!caids) continue;
						do {
							cerr << "checking " << *caids << endl;
							if ( sysid == *caids) {
								int l = length + cam_length[Slot];
								if (l <= BLEN) {
									memcpy(cam_buf+ Slot*BLEN + 
									       cam_length[Slot], 
									       buffer+12, length);
									cam_length[Slot] += length;
								}
								done=1;
							} else caids++;
							count++;
						} while (!done && *caids && count<MAXCASYSTEMIDS);
					} 
				}
			}
		}

		if (conn2 >=0 && FD_ISSET(conn2, &rfds)){
			uint8_t sl=0;
			CamPacket cp(conn2);
			if (cp.preceive()<0){
				close(conn2);
				conn2=-1;
				cerr << "CAM MENU: error, closing connection" << endl;
				mtime = time(0)+MENUTIMEOUT;
				menustate = MENU_NONE;
				if (!simple_ci && ciMenu) ciMenu->Cancel();
				if (simple_ci) ci_cancel_menu(cafd,0);
				continue;
			}
			

			if (cp.type() == CAM_RESET){
				if ( (!simple_ci && ciHandler && 
				      ciHandler->Reset(0)) || 
				     (simple_ci && !ci_reset(cafd,0))){
					close(conn2);
					conn2=-1;
					close(conn1);
					conn1=-1;
					
					cerr << "Resetting CAM" << endl;
					mtime = 0;
					menustate = MENU_NONE;
					state = SEARCH;
					continue;
				}
			}
				
			if (cp.type() == CMENU_CLOSE){
				close(conn2);
				conn2=-1;
				cerr << "Closing Menu Connection" << endl;
				mtime = time(0)+MENUTIMEOUT;
				menustate = MENU_NONE;
				if(!simple_ci && ciMenu) ciMenu->Cancel();
				if (simple_ci) ci_cancel_menu(cafd,0);
				continue;
			}
			
			switch (menustate){ 
			
			case MENU_NONE:
			{
				if (cp.type() != CMENU_SELECT) break;
				sl = cp.length();
				sl -= 48;
				if (!sl) break;
				if (sl<= numslot && 
				    ((!simple_ci && ciHandler->EnterMenu(sl-1))||
				     (simple_ci && !ci_enter_menu(cafd,sl-1)))){
					cerr << "Enter Menu" << endl;
					mtime = time(0)+MENUTIMEOUT;
				} else {
					char errmsg[21];
					strncpy(errmsg,"Can't open CAM menu!",20);
					errmsg[20]=0;
					CamPacket cp(conn2, CMENU_ERROR, 21, (uint8_t *)errmsg);
					cp.psend();
					cerr << errmsg << endl;
				}
			}
			break;
				
			case MENU_START:
			{
				if (cp.type() != CMENU_SELECT) break;
				sl = cp.length();

				sl -= 48;
				if (!simple_ci && ciMenu){ 
					if (ciMenu->Selectable() && sl <= mrange && sl>0){
						ciMenu->Select((uint8_t)(sl-1));
					} else {
						ciMenu->Cancel();
					}
				} else if (simple_ci){
					if (sl <= mrange && sl>0){
						ci_menu_select(cafd,0,(uint8_t)(sl-1));
					} else {
						ci_cancel_menu(cafd,0);
					}
					

				}
				mtime = time(0)+MENUTIMEOUT;
			}
			break;
			
			case MENU_ENQUIRY:
			{
				if (cp.type() != CENQUIRY_REPLY) break;

				input = new char [EnLength+1];
				memset(input,0, EnLength+1);
				
				if ( cp.length() == EnLength){ 
					memcpy(input, cp.Payload(), EnLength);
					if (input[0] == 'q'){
						ciEnquiry->Cancel();
					} else {
						if (ciEnquiry) ciEnquiry->Reply(input);
					}
					menustate = MENU_START;
					delete [] input;
				}
			}
			break;
			}
		}

		if (simple_ci){

			if(state==CAM){
				char devname[80];
				sprintf(devname,DEMUX_DEV,adapter,minor);
				int fd = open(devname, O_RDWR);
				set_sid = current_sid;
				ci_enable_pnr(fd, cafd, 0, current_sid); 
				close(fd);
				state = NONE;
			}

			uint8_t buf[256];
			if (ci_getmsg(cafd, 0, buf)>=0){
				CamPacket cp;
				mtime = 0;
				if (strlen((char *)buf)>1)
					fprintf(stderr,"%s\n",(char *)buf);
				
			}

		}

		if (!simple_ci && ciHandler) {
			if (mtime && mtime < time(0)){
				mtime = 0;
				menustate = MENU_NONE;
			}
			if (ciHandler->HasUserIO()) {
				CamPacket cp;
				ciMenu = NULL;
				ciMenu = ciHandler->GetMenu();
				mtime = 0;
				if (ciMenu){
					if (ciMenu->TitleText()){
						cp.init(conn2, CMENU_TITLE, strlen(ciMenu->TitleText()),
							(uint8_t *)ciMenu->TitleText());
						cp.psend();
						cout << ciMenu->TitleText() << endl;
						if (ciMenu->SubTitleText()){
							cp.init(conn2, CMENU_SUBTITLE, 
								strlen(ciMenu->SubTitleText()),
								(uint8_t *)ciMenu->SubTitleText());
							cp.psend();

							cout << ciMenu->SubTitleText() << endl;
						}
					} else {
						cp.init(conn2, CMENU_TITLE, 0, NULL); 
						cp.psend();
						cout << "CAM MENU" << endl;
					}
					mrange = ciMenu->NumEntries();
					cout << endl;
					cp.init(conn2, CMENU_NENTRY, mrange, NULL); 
					cp.psend();

					for (int i = 0; i < mrange; i++){
						char dummy[80];

						strncpy(dummy,ciMenu->Entry(i), 80); 
						cp.init(conn2, CMENU_ENTRY, sizeof(dummy), 
							(uint8_t *)(dummy), i);
						cp.psend();
						cout << i+1 << ") " << dummy << endl;

					}
					cout << endl;
					if (ciMenu->BottomText()){
						cp.init(conn2, CMENU_BOTTOMTEXT, 
							strlen(ciMenu->BottomText()),
							(uint8_t *)ciMenu->BottomText());
						cp.psend();
						
						cout << ciMenu->BottomText() << endl << endl;
					}
					menustate = MENU_START;
					cout << "Enter 0 to exit" << endl;
				} else {
					ciEnquiry = ciHandler->GetEnquiry();
					if (ciEnquiry) {
						EnLength = ciEnquiry->ExpectedLength();
						cp.init(conn2, CENQUIRY_LENGTH, EnLength, NULL);
						cp.psend();
						
						if(ciEnquiry->Text()){
							cp.init(conn2, CENQUIRY_TEXT, 
								strlen(ciEnquiry->Text()),
								(uint8_t *)ciEnquiry->Text());
							cp.psend();
						
							cout << "Enquiry: " << ciEnquiry->Text() << endl;
						} else {
							cp.init(conn2, CENQUIRY_TEXT, 0, NULL); 
							cp.psend();
							cout << "CAM Enquiry" << endl;
						}
						menustate = MENU_ENQUIRY;
						cout << "Enter key of length " << EnLength << " or q to exit" << endl;
						enread = 0;
					}
				}
			} 


			if (ciHandler->Process()) {
				if (state == CAM) {
					for (int Slot = 0; Slot < numslot; Slot++) {
						if (cam_length[Slot] > 0) {
							cCiCaPmt CaPmt(current_sid);
							CaPmt.AddCaDescriptor(cam_length[Slot], 
									      cam_buf+Slot*BLEN);
							if (current_vpid) 
								CaPmt.AddPid(current_vpid);
							if (current_apid1)
								CaPmt.AddPid(current_apid1);
							if (current_apid2)
								CaPmt.AddPid(current_apid2);
							if (current_dpid1)
								CaPmt.AddPid(current_dpid1);
							if (ciHandler->SetCaPmt(
								    CaPmt,Slot)) {
								state = NONE;
								cerr << "state: NONE" << endl;
								cam_length[Slot]=0;
								memset(cam_buf, 0,Slot*BLEN);
								timeout=1000;
							}
						}
					}
				} else if(!ciHandler->connected() && time(0)-startt > TIMEOUT) {
					cerr << "No working CAM found within " << TIMEOUT << " seconds." << endl;
					if(conn1>=0)
						close(conn1);
					if(ip_sock1>=0)
						close(ip_sock1);
					if(conn2>=0)
						close(conn2);
					if(ip_sock2>=0)
						close(ip_sock2);
					exit(0);
				}
			}
		}
	}
}
