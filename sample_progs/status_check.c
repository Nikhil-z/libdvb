#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <linux/dvb/frontend.h>


#define FRONT_DEV "/dev/dvb/adapter%d/frontend%d"

void chck_frontend_state (int fefd)
{
	fe_status_t status=0;
	uint16_t snr=0, strength=0;
	uint32_t ber=0, u_blocks=0;
	int err=0;
	struct dvb_frontend_parameters tune;
	fe_info_t info;
	int type;
	int freq;
	char *fec[]={"NONE","1/2","2/3","3/4","4/5","5/6","6/7","7/8","8/9","AUTO"};
	char *mod[]={"QPSK","QAM16","QAM32","QAM64","QAM128","QAM256","QAM_AUTO"};
	char *band[]={"8 MHz","7 MHz","6 MHz","AUTO"};
	char *guard[]={"1/32","1/16","1/8","1/4","AUTO"};
	char *hierarchy[]={"NONE","1","2","3","AUTO"};
	char *trans[]={"mode2","mode8","AUTO"};
	

	err = ioctl(fefd, FE_READ_STATUS, &status);
	err +=ioctl(fefd, FE_GET_INFO, &info);
	err +=ioctl(fefd, FE_READ_SNR, &snr);
	err +=ioctl(fefd, FE_READ_SIGNAL_STRENGTH, &strength);
	err +=ioctl(fefd, FE_READ_BER, &ber);
	err +=ioctl(fefd, FE_READ_UNCORRECTED_BLOCKS, &u_blocks);
	
	if (err){
		printf("Error reading status %d\n", err);
		perror("frontend");
	} else 
		printf ("status %02x \nsignal %04x \nsnr %04x \nber %08x \nunc %08x \n\n",
			status, strength, snr, ber, u_blocks);

	type=info.type;
	err +=ioctl(fefd, FE_GET_FRONTEND, &tune);
	freq=tune.frequency;
	
	switch(type){
	case FE_QPSK:
		printf("QPSK\n");
		printf("freq %d kHz (%.3f MHz or %.3f MHz)\n",freq,freq/1000.0+9750.0,
		       freq/1000.0+10600.0);
		switch (tune.inversion){
		case INVERSION_OFF:
			printf("inv off\n");
			break;
		case INVERSION_ON:
			printf("inv on\n");
			break;
		case INVERSION_AUTO:
			printf("inv auto\n");
			break;
		}
		printf("sr %d Sym/s\n",tune.u.qpsk.symbol_rate);
		printf("fec %s\n",fec[tune.u.qpsk.fec_inner]);
		break;

	case FE_QAM:
		printf("QAM\n");
		printf("freq %d\n Hz",freq);
		switch (tune.inversion){
		case INVERSION_OFF:
			printf("inv off\n");
			break;
		case INVERSION_ON:
			printf("inv on\n");
			break;
		case INVERSION_AUTO:
			printf("inv auto\n");
			break;
		}
		printf("sr %d Sym/s\n",tune.u.qam.symbol_rate);
		printf("fec %s\n",fec[tune.u.qam.fec_inner]);
		printf("modulation %s\n",mod[tune.u.qam.modulation]);
		
		break;

	case FE_OFDM:
		printf("OFDM\n");
		printf("freq %d\n Hz",freq);
		switch (tune.inversion){
		case INVERSION_OFF:
			printf("inv off\n");
			break;
		case INVERSION_ON:
			printf("inv on\n");
			break;
		case INVERSION_AUTO:
			printf("inv auto\n");
			break;
		}
		printf("bandwidth %s\n",band[tune.u.ofdm.bandwidth]);
		printf("fecHP %s\n",fec[tune.u.ofdm.code_rate_HP]);
		printf("fecLP %s\n",fec[tune.u.ofdm.code_rate_LP]);
		printf("constellation %s\n",mod[tune.u.ofdm.constellation]);
		printf("transmit %s\n",trans[tune.u.ofdm.transmission_mode]);
		printf("guard %s\n",guard[tune.u.ofdm.guard_interval]);
		printf("hierarchy %s\n",hierarchy[tune.u.ofdm.hierarchy_information]);

		break;

	}

}

int main(int argc, char **argv)
{
	int adapter=0;
	int minor=0;
	char devname[80];        
	int fd_front;
	int c;

	memset(devname, 0, 80);

        for (;;) {
                if (-1 == (c = getopt(argc, argv, "a:")))
                        break;
                switch (c) {
		case 'a':
			adapter = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'm':
			minor = strtol(optarg,(char **) NULL, 0);
                        break;
		}
	}

	sprintf(devname,FRONT_DEV,adapter,minor);
	if ((fd_front=open(devname, O_RDONLY))<0){
		perror("frontend");
		printf("Error opening %s\n",devname);
                exit(1);
	}
	chck_frontend_state (fd_front);

	return 0;
}
