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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/dvb/ca.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "cam_menu.hh"
#include "DVB.hh"

struct termios stored_settings;

void set_keypress()
{
	struct termios new_settings;
  
	tcgetattr(STDIN_FILENO,&stored_settings);
	tcgetattr(STDIN_FILENO,&new_settings);

	new_settings.c_iflag = 0;
	new_settings.c_lflag &= ~(ICANON | ECHO);
	new_settings.c_cc[VTIME] = 0;
	new_settings.c_cc[VMIN] = 0;
	
	tcsetattr(STDIN_FILENO, TCSANOW,&new_settings);
}

void reset_keypress()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &stored_settings);
}


#define MENU_SOCK 4712 

void byebye(int i, void *f)
{
	int fd = (int) f;

	CamPacket cp(fd, CMENU_CLOSE, 0, NULL);
	cp.psend();
	close(fd);
	reset_keypress();
}

int main(int argc, char **argv)
{
	int c;
	char *sockname=0;
	int menu_sock = MENU_SOCK;
	int handler_fd;
	struct pollfd pfd[2];

	for (;;) {
                if (-1 == (c = getopt(argc, argv, "s:u:")))
                        break;
                switch (c) {
		case 's':
			menu_sock=strtol(optarg,(char **) NULL, 0);
			break;

		case 'u':
			sockname = strdup(optarg);
			break;
	}
	}

	if(sockname)
		handler_fd = udp_client_connect(sockname);
	else
		handler_fd = tcp_client_connect("localhost", menu_sock);
	

	on_exit(byebye, (void *)handler_fd);
	pfd[0].fd = STDIN_FILENO;
        pfd[0].events = POLLIN;
	pfd[1].fd = handler_fd;
        pfd[1].events = POLLIN;

	set_keypress();
	while(1){
		if (poll(pfd,2,100)>0){
			if ( pfd[0].revents & POLLIN ){
				uint8_t sl;

				int r=read(STDIN_FILENO, &sl, 1);
				if (r==1){
					if (sl == 'r'){
						CamPacket cp(handler_fd, CAM_RESET, sl, NULL);
						cp.psend();
						close(handler_fd);
						if(sockname)
							handler_fd = udp_client_connect(sockname);
						else
							handler_fd = tcp_client_connect("localhost", menu_sock);
					} else if (sl == 'q'){
						cerr << "closing down" << endl;
						exit(0);
					} else {
						CamPacket cp(handler_fd, CMENU_SELECT, sl, NULL);
						cp.psend();
					}

					cout << char(sl) << endl;
				}
				
			}
			
			if ( pfd[1].revents & POLLIN ){
				CamPacket cp(handler_fd);
				cp.preceive();
				
				switch(cp.type()){
				case CMENU_TITLE: 
				case CMENU_SUBTITLE: 
				case CMENU_BOTTOMTEXT: 
				case CMENU_ERROR: 
				case CENQUIRY_TEXT: 
					if (cp.length()) cout << cp.Payload() << endl;
					break;
				case CMENU_ENTRY: 
					if (cp.length()) cout << cp.extra()+1 << ") "
							      << cp.Payload() << endl;
					break;
				case CMENU_NENTRY: 
					cout << cp.length() << " entries" << endl;
					break;
				case CENQUIRY_BLIND:
					break;
				case CENQUIRY_LENGTH: 
					cout << cp.length() << " enquiry length" << endl;
					break;
				case CENQUIRY_REPLY: 
				case CMENU_SELECT:
					break;
				}
			}
		}
	}
}
