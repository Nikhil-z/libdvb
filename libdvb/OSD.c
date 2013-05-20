#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <linux/dvb/osd.h>

int OSDClose(int dev)
{
        osd_cmd_t dc;

        dc.cmd=OSD_Close;
        return ioctl(dev, OSD_SEND_CMD, &dc);
} 


int OSDOpen(int dev, int x0, int y0, int x1, int y1,
	     int BitPerPixel, int mix) 
{
        osd_cmd_t dc;

        dc.cmd=OSD_Open;
	dc.x0=x0;
	dc.y0=y0;
	dc.x1=x1;
	dc.y1=y1;
	dc.data=0;
	dc.color=BitPerPixel&0xf;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDShow(int dev) 
{
        osd_cmd_t dc;

        dc.cmd=OSD_Show;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDHide(int dev) 
{
        osd_cmd_t dc;

        dc.cmd=OSD_Hide;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDClear(int dev) 
{
        osd_cmd_t dc;

        dc.cmd=OSD_Clear;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDFill(int dev, int color) 
{
        osd_cmd_t dc;

        dc.cmd=OSD_Fill;
	dc.color=color;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDSetColor(int dev, int color, int r, int g, int b, int op) 
{
        osd_cmd_t dc;

        dc.cmd=OSD_SetColor;
	dc.color=color;
	dc.x0=r;
	dc.y0=g;
	dc.x1=b;
	dc.y1=op;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDText(int dev, int x, int y, int size, int color, const char *text) 
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_Text;
	dc.x0=x;
	dc.y0=y;
	dc.x1=size; // fontsize (1, 2 or 3)
	dc.data=(char *)text;
	dc.color=color;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDSetPalette(int dev, int first, int last, unsigned char *data) 
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_SetPalette;
	dc.color=first;
	dc.x0=last;
	dc.data=data;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDSetTrans(int dev, int trans)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_SetTrans;
	dc.color=trans;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDSetPixel(int dev, int x, int y, unsigned int color)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_SetPixel;
	dc.x0=x;
	dc.y0=y;
	dc.color=color;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDGetPixel(int dev, int x, int y)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_GetPixel;
	dc.x0=x;
	dc.y0=y;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDSetRow(int dev, int x, int y, int x1, unsigned char *data)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_SetRow;
	dc.x0=x;
	dc.y0=y;
	dc.x1=x1;
	dc.data=data;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDSetBlock(int dev, int x, int y, int x1, int y1, int inc, 
		unsigned char *data)
{
	int size;
        osd_cmd_t dc;
	

	size=
	dc.cmd=OSD_SetBlock;
	dc.x0=x;
	dc.y0=y;
	dc.x1=x1;
	dc.y1=y1;
	dc.color=inc;
	dc.data=data;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDFillRow(int dev, int x, int y, int x1, int color)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_FillRow;
	dc.x0=x;
	dc.y0=y;
	dc.x1=x1;
	dc.color=color;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDFillBlock(int dev, int x, int y, int x1, int y1, int color)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_FillBlock;
	dc.x0=x;
	dc.y0=y;
	dc.x1=x1;
	dc.y1=y1;
	dc.color=color;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDLine(int dev, int x, int y, int x1, int y1, int color)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_Line;
	dc.x0=x;
	dc.y0=y;
	dc.x1=x1;
	dc.y1=y1;
	dc.color=color;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDQuery(int dev)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_Query;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDSetWindow(int dev, int win)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_SetWindow;
	dc.x0=win;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

int OSDMoveWindow(int dev, int x, int y)
{
        osd_cmd_t dc;
	
	dc.cmd=OSD_MoveWindow;
	dc.x0=x;
	dc.y0=y;
	return ioctl(dev, OSD_SEND_CMD, &dc);
}

