#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <stdint.h>
#include <sys/time.h>


#include <linux/dvb/frontend.h>
#include <linux/dvb/ca.h>
#include <linux/dvb/dmx.h>
// Application Object Tags:

#define AOT_NONE                    0x000000
#define AOT_PROFILE_ENQ             0x9F8010
#define AOT_PROFILE                 0x9F8011
#define AOT_PROFILE_CHANGE          0x9F8012
#define AOT_APPLICATION_INFO_ENQ    0x9F8020
#define AOT_APPLICATION_INFO        0x9F8021
#define AOT_ENTER_MENU              0x9F8022
#define AOT_CA_INFO_ENQ             0x9F8030
#define AOT_CA_INFO                 0x9F8031
#define AOT_CA_PMT                  0x9F8032
#define AOT_CA_PMT_REPLY            0x9F8033
#define AOT_TUNE                    0x9F8400
#define AOT_REPLACE                 0x9F8401
#define AOT_CLEAR_REPLACE           0x9F8402
#define AOT_ASK_RELEASE             0x9F8403
#define AOT_DATE_TIME_ENQ           0x9F8440
#define AOT_DATE_TIME               0x9F8441
#define AOT_CLOSE_MMI               0x9F8800
#define AOT_DISPLAY_CONTROL         0x9F8801
#define AOT_DISPLAY_REPLY           0x9F8802
#define AOT_TEXT_LAST               0x9F8803
#define AOT_TEXT_MORE               0x9F8804
#define AOT_KEYPAD_CONTROL          0x9F8805
#define AOT_KEYPRESS                0x9F8806
#define AOT_ENQ                     0x9F8807
#define AOT_ANSW                    0x9F8808
#define AOT_MENU_LAST               0x9F8809
#define AOT_MENU_MORE               0x9F880A
#define AOT_MENU_ANSW               0x9F880B
#define AOT_LIST_LAST               0x9F880C
#define AOT_LIST_MORE               0x9F880D
#define AOT_SUBTITLE_SEGMENT_LAST   0x9F880E
#define AOT_SUBTITLE_SEGMENT_MORE   0x9F880F
#define AOT_DISPLAY_MESSAGE         0x9F8810
#define AOT_SCENE_END_MARK          0x9F8811
#define AOT_SCENE_DONE              0x9F8812
#define AOT_SCENE_CONTROL           0x9F8813
#define AOT_SUBTITLE_DOWNLOAD_LAST  0x9F8814
#define AOT_SUBTITLE_DOWNLOAD_MORE  0x9F8815
#define AOT_FLUSH_DOWNLOAD          0x9F8816
#define AOT_DOWNLOAD_REPLY          0x9F8817
#define AOT_COMMS_CMD               0x9F8C00
#define AOT_CONNECTION_DESCRIPTOR   0x9F8C01
#define AOT_COMMS_REPLY             0x9F8C02
#define AOT_COMMS_SEND_LAST         0x9F8C03
#define AOT_COMMS_SEND_MORE         0x9F8C04
#define AOT_COMMS_RCV_LAST          0x9F8C05
#define AOT_COMMS_RCV_MORE          0x9F8C06



struct ca_info {
	int sys_num;
	uint16_t sys_id[256];
	char app_name[256];
};

static void dump(uint8_t *D, int len)
{
        int i;

        for (i=0; i<len; i++)
                printf("%02x ", D[i]);
        printf("\n");
}

int convert_desc(struct ca_info *cai, 
		 uint8_t *out, uint8_t *buf, int dslen, uint8_t cmd)
{
 	int i, j, dlen, olen=0;

	out[2]=cmd;
	for (i=0; i<dslen; i+=dlen) {
		dlen=buf[i+1]+2;
		if ((buf[i]==9)&&(dlen>2)&&(dlen+i<=dslen)) {
			for (j=0; j<cai->sys_num; j++)
				if (cai->sys_id[j]==((buf[i+2]<<8)|buf[i+3]))
					break;
			if (j==cai->sys_num)
				continue;
			memcpy(out+olen+3, buf+i, dlen);
			olen+=dlen;
		}
	}
	olen=olen?olen+1:0;
	out[0]=(olen>>8);
	out[1]=(olen&0xff);
	return olen+2;
}

static int convert_pmt(struct ca_info *cai, uint8_t *out, uint8_t *buf, 
		       uint8_t list, uint8_t cmd)
{
	int slen, dslen, o, i;

	slen=(((buf[1]&0x03)<<8)|buf[2])+3;
	out[0]=list; 
	out[1]=buf[3];
	out[2]=buf[4];
	out[3]=buf[5];
	dslen=((buf[10]&0x0f)<<8)|buf[11];
	o=4+convert_desc(cai, out+4, buf+12, dslen, cmd);
	for (i=dslen+12; i<slen-9; i+=dslen+5) {
		dslen=((buf[i+3]&0x0f)<<8)|buf[i+4];
		if ((buf[i]==0)||(buf[i]>4))
			continue;
		out[o++]=buf[i];
		out[o++]=buf[i+1];
		out[o++]=buf[i+2];
		o+=convert_desc(cai, out+o, buf+i+5, dslen, cmd);
	}
	return o;
}

int get_sec(int fd, uint8_t *buf, uint16_t ppid, uint8_t table)
{
	struct dmx_sct_filter_params f;

	memset(&f, 0, sizeof(f));
	f.pid                       = ppid;
	f.filter.filter[0]          = table;
	f.filter.mask[0]            = 0xFF;
	f.timeout                   = 2000;
	f.flags                     = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_FILTER, &f) < 0)  
		return -1;
	return read(fd, buf, 4096);
}

static uint16_t get_pmt_pid(uint8_t *pat, uint16_t pnr)
{
	int slen=(((pat[1]&0x03)<<8)|pat[2])+3, i;
	
	for (i=8; i<slen-2; i+=4)
		if (pnr==((pat[i]<<8)|pat[i+1]))
			return ((0x1f&pat[i+2])<<8)|pat[i+3];
	return 0;
}

static uint16_t get_pmt(int fd, uint16_t pnr)
{
	uint8_t buf[4096], last=0xff;
	uint16_t pmt;
	int len;
	
	do {
		if ((len=get_sec(fd, buf, 0x00, 0x00))<0)
			break;
		if (last==0xff)
			last=(buf[6]+buf[7])%(buf[7]+1);
		if ((pmt=get_pmt_pid(buf, pnr)))
			return pmt;
	} while(buf[6]!=last);
	return 0;
}

void ci_get_info(int fd)
{
	ca_slot_info_t info;

	info.num=0;
	ioctl(fd, CA_GET_SLOT_INFO, &info);
	
	printf("INFO type=%d, flags=0x%02x\n", info.type, info.flags);

}

static int ci_read_length(uint8_t **lf)
{
	int length=*(*lf)++;

	if (length&0x80) {
		int i, n=length&0x7f;
		for (i=0, length=0; i<n; i++) 
			length=(length<<8)|*(*lf)++;
	}	
	return length;
}

static int ci_write_length(uint8_t **lf, int length)
{
	int i, n=1;

	if (length<128) 
		*(*lf)++=length;
	else {
		for (i=sizeof(int)-1; i>=0; i--)
			if ((length>>(i<<3))&0xff)
				break;
		for (n+=i, *(*lf)++=n|0x80; i>=0; i--)
			*(*lf)++=(length>>(i<<3))&0xff;
	}
	return n;
}

int ci_send_obj(int fd, uint32_t tag, uint8_t *buf, uint16_t len)
{
	ca_msg_t msg;
	uint8_t *p=msg.msg;

	*p++=(tag>>16)&0xff;
	*p++=(tag>>8)&0xff;
	*p++=tag&0xff;
	ci_write_length(&p, len);
	if (buf)
		memcpy(p, buf, len);
	msg.index=0;
	msg.type=CA_CI;
	msg.length=p-msg.msg+len;
	dump(msg.msg, msg.length);
	return ioctl(fd, CA_SEND_MSG, &msg);
}

int ci_getmsg(int fd, int slot, uint8_t *buf)
{
	ca_msg_t msg;
	int res;

	res=ioctl(fd, CA_GET_MSG, &msg);
	if (res<0)
		return res;
	memcpy(buf, &msg.msg, msg.length);
	dump(buf, msg.length);
	return msg.length;
}

int ci_get_ca_info(int fd, int slot, struct ca_info *cai)
{
	uint8_t buf[256], *p=buf+3;
	int len, i;

	ci_send_obj(fd, AOT_CA_INFO_ENQ, NULL, 0);
	if ((len=ci_getmsg(fd, slot, buf))<=8)
		return -1;
	printf("CA systems : ");
	len=ci_read_length(&p);
	cai->sys_num=p[0];
	for (i=0; i<cai->sys_num; i++) {
		cai->sys_id[i]=(p[1+i*2]<<8)|p[2+i*2];
		printf("%04x ", cai->sys_id[i]);
	}
	printf("\n");
	return 0;
}

int ci_get_app_info(int fd, int slot,struct ca_info *cai)
{
	uint8_t answ[256], *p=answ+3;
	int len;

	//ci_send(fd, 0x01, 0x0000, NULL, 0);
	ci_send_obj(fd, AOT_APPLICATION_INFO_ENQ, NULL, 0);
	if ((len=ci_getmsg(fd, slot, answ))<=8)
		return -1;
	answ[len-1]=0;
	len=ci_read_length(&p);
	strncpy(cai->app_name, p+5, 256);
	printf("module: %s, app type %02x, app manf %04x\n",
	       p+5, p[0], (p[1]<<8)|p[2]);
	return 0;
}


int ci_enable_pnr(int fd, int ci, int slot, uint16_t pnr) 
{
	uint8_t buf[4096], pmt[4096];
	struct ca_info cai;
	uint16_t pid;
	int len;

	ci_get_info(ci);
	ci_get_app_info(ci,slot, &cai);
	ci_get_ca_info(ci, slot, &cai);
	if (!(pid=get_pmt(fd, pnr)))
		return -1;
	if ((len=get_sec(fd, buf, pid, 0x02))<0)
		return len;
	len=convert_pmt(&cai, pmt, buf, 3, 1);
	printf("CA_PMT[%d] = ", len);
	dump(pmt, len);
	ci_send_obj(ci, AOT_CA_PMT, pmt, len);
	return 0;
}

int ci_reset(int fd, int slot)
{
	struct ca_info cai;

	ci_get_info(fd);
	ci_get_app_info(fd,slot, &cai);
	ci_get_ca_info(fd, slot, &cai);
	if (ioctl(fd, CA_RESET, 1 << slot) != -1) {
		return 0;
	} else  return -1;
}

int ci_send_menu_answer(int fd, int slot, uint8_t answ)
{
	return ci_send_obj(fd, AOT_MENU_ANSW, &answ, 1);

}

int ci_cancel_menu(int fd, int slot)
{
	return ci_send_menu_answer(fd,slot,0);
}

int ci_menu_select(int fd, int slot, uint8_t sl)
{
	return ci_send_menu_answer(fd,slot,sl);
}

int ci_enter_menu(int fd, int slot)
{
	int re;
	ci_send_obj(fd, AOT_CLOSE_MMI, NULL, 0);
        re= ci_send_obj(fd, AOT_ENTER_MENU, NULL, 0);
	sleep(3);
	return re;
}

