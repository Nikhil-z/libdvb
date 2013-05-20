/*
 * cam_menu.hh
 *
 */
#include <sys/types.h>
#include <sys/socket.h>

#ifndef __CAM_MENU_HH
#define __CAM_MENU_HH
#define MENU_SOCK 4712 



typedef 
enum { CMENU_TITLE, 
       CMENU_SUBTITLE, 
       CMENU_BOTTOMTEXT, 
       CMENU_ENTRY, 
       CMENU_NENTRY, 
       CENQUIRY_TEXT, 
       CENQUIRY_BLIND,
       CENQUIRY_LENGTH, 
       CENQUIRY_REPLY, 
       CMENU_SELECT,
       CMENU_ERROR,
       CMENU_CLOSE,
       CAM_RESET
} cpacket_type;

typedef
struct campacket_s{
	cpacket_type type;
	int length;
	int extra;
} campacket;

class CamPacket{
	campacket cp;
	uint8_t *payload;
	int fd;
	int clearp;

	int read_payload(){
		int count = 0;
		int length = cp.length;
		int to=0;

		if (!payload){
			payload = new uint8_t [length+1];
			clearp = 1;
			memset(payload,0,length+1);
		}

		while(count < length && to<10){
			int re;
			if ((re=read(fd, (char *)(payload+count), length-count))>=0) count+=re;
			else perror("read_payload");
			to++;
		}
		if (to==10) return -1;
		else return 0;
	}

	int read_pack(){
		int count = 0;
		int length = sizeof(campacket);
		int to=0;

		while(count < length && to<10){
			int re;
			if ((re=read(fd, (char *)(&cp+count), length-count))>=0) count+=re;
			else perror("read_pack");
			to++;
		}
		if (to==10) return -1;
		else return 0;
	}

public:
	void init(int f, int t, int  length, uint8_t *p, int e=-1){
		cp.type = cpacket_type(t);
		cp.length = length;
		payload = p;
		fd = f;
		clearp = 0;
		cp.extra = e;
	}

	void clear(){
		init(0, 0, 0, NULL);
		if (clearp) delete [] payload;
	}

	~CamPacket(){
		if (clearp) delete [] payload;
	}

	CamPacket(int f){
		init(f, 0, 0, NULL);
	}

	CamPacket(){
		init(0, 0, 0, NULL);
	}

	CamPacket(int f, int t, int  length, uint8_t *p, int e=-1){
		init(f, t, length, p, e);
	}


	int psend(){
		if (fd <0) return -1;
		int sent = send(fd,(void *)&cp,sizeof(campacket),0);

		if(sent < 0){
			perror("send");
			return sent;
		}

		switch (cp.type){
		case CMENU_TITLE: 
		case CMENU_SUBTITLE: 
		case CMENU_BOTTOMTEXT: 
		case CMENU_ENTRY: 
		case CENQUIRY_TEXT: 
		case CENQUIRY_REPLY: 
		case CMENU_ERROR:
			sent = send(fd, payload, cp.length, 0);
			if(sent < 0){
				perror("send");
				return -sent;
			}
			break;
		case CMENU_NENTRY: 
		case CENQUIRY_BLIND:
		case CENQUIRY_LENGTH: 
		case CMENU_SELECT:
		case CAM_RESET:
		case CMENU_CLOSE:
			break;
		}
		return 0;
	}

	int preceive(){
		if (fd<0) return -1;
		if (read_pack()<0) return -1;

		switch (cp.type){
		case CMENU_TITLE: 
		case CMENU_SUBTITLE: 
		case CMENU_BOTTOMTEXT: 
		case CMENU_ENTRY: 
		case CENQUIRY_TEXT: 
		case CENQUIRY_REPLY: 
		case CMENU_ERROR:
			if (cp.length)
			  if (read_payload()<0) return -1;
		break;
		case CMENU_NENTRY: 
		case CENQUIRY_BLIND:
		case CENQUIRY_LENGTH: 
		case CMENU_SELECT:
		case CMENU_CLOSE:
		case CAM_RESET:
			break;

		}
		return 0;
	}

	int type(){ return int(cp.type);}
	int length(){ return int(cp.length);}
	int extra(){ return int(cp.extra);}
	char *Payload(){ return (char *)payload;}

};

#endif //__CAM_MENU_HH
