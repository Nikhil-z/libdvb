/* 
 * show_dvb_image.c
 *
 * Copyright (C) 2004 Marcus Metzler <mocm@metzlerbros.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include <linux/dvb/video.h>

int videoPlay(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_PLAY) < 0)){
		perror("VIDEO PLAY: ");
		return -1;
	}

	return 0;
}

int videoStillPicture(int fd,  struct video_still_picture *sp)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_STILLPICTURE, sp) < 0)){
		perror("VIDEO STILLPICTURE: ");
		return -1;
	}

	return 0;
}

void load_iframe(int filefd, int fd,int ti)
{
	struct stat st;
	struct video_still_picture sp;

	fstat(filefd, &st);
	
	sp.iFrame = (char *) malloc(st.st_size);
	sp.size = st.st_size;
	printf("I-frame size: %d\n", sp.size);
	
	if(!sp.iFrame) {
		printf("No memory for I-Frame\n");
		return;
	}

	printf("read: %d bytes\n",read(filefd,sp.iFrame,sp.size));
	videoStillPicture(fd,&sp);

	sleep(ti);
	videoPlay(fd);
}

int main(int argc, char **argv)
{
	int fd;
	int filefd;
	int ti=3;

	if (argc < 2){
	  fprintf(stderr,"usage: %s <filename> [secs]\n",argv[0]);
	    return -1;
	}

	if (argc == 3) ti = strtol(argv[2],(char **) NULL, 0);

	if ( (filefd = open(argv[1],O_RDONLY)) < 0){
		perror("File open:");
		return -1;
	}
	if((fd = open("/dev/dvb/adapter0/video0",O_RDWR|O_NONBLOCK)) < 0){
		perror("VIDEO DEVICE: ");
		return -1;
	}
	    
	
	load_iframe(filefd, fd,ti);
	close(fd);
	close(filefd);
	return 0;
}

