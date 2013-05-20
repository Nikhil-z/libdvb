#include <DVB.hh>

static const char *vdr_inv_name [] = {
        "0",
        "1",
        "999"
};

static const char *vdr_fec_name [] = {
        "0",
        "12",
        "23",
        "34",
        "45",
        "56",
        "67",
        "78",
        "89",
        "999"
};

static const char *vdr_qam_name [] = {
        "0",
        "16",
        "32",
        "64",
        "128",
        "256",
        "999"
};


static const char *vdr_bw_name [] = {
        "8",
        "7",
        "6",
        "999"
};


static const char *vdr_mode_name [] = {
        "2",
        "8",
        "999"
};

static const char *vdr_guard_name [] = {
        "32",
        "16",
        "8",
        "4",
        "999"
};


static const char *vdr_hierarchy_name [] = {
        "0",
        "1",
        "2",
        "4",
        "999"
};


static const char *fec_name [] = {
        "FEC_NONE",
        "FEC_1_2",
        "FEC_2_3",
        "FEC_3_4",
        "FEC_4_5",
        "FEC_5_6",
        "FEC_6_7",
        "FEC_7_8",
        "FEC_8_9",
        "FEC_AUTO"
};

static const char *inv_name [] = {
        "INVERSION_OFF",
        "INVERSION_ON",
        "INVERSION_AUTO"
};

static const char *qam_name [] = {
        "QPSK",
        "QAM_16",
        "QAM_32",
        "QAM_64",
        "QAM_128",
        "QAM_256",
        "QAM_AUTO"
};


static const char *bw_name [] = {
        "BANDWIDTH_8_MHZ",
        "BANDWIDTH_7_MHZ",
        "BANDWIDTH_6_MHZ",
        "BANDWIDTH_AUTO"
};


static const char *mode_name [] = {
        "TRANSMISSION_MODE_2K",
        "TRANSMISSION_MODE_8K",
        "TRANSMISSION_MODE_AUTO"
};

static const char *guard_name [] = {
        "GUARD_INTERVAL_1_32",
        "GUARD_INTERVAL_1_16",
        "GUARD_INTERVAL_1_8",
        "GUARD_INTERVAL_1_4",
        "GUARD_INTERVAL_AUTO"
};


static const char *hierarchy_name [] = {
        "HIERARCHY_NONE",
        "HIERARCHY_1",
        "HIERARCHY_2",
        "HIERARCHY_4",
        "HIERARCHY_AUTO"
};

static const fe_code_rate_t ftab [8] = {
        FEC_AUTO, FEC_1_2, FEC_2_3, FEC_3_4,
        FEC_5_6, FEC_7_8, FEC_NONE, FEC_NONE
};

static const fe_modulation_t qtab [6] = {
        QAM_AUTO, QAM_16, QAM_32, QAM_64, QAM_128, QAM_256
};


void dvb2txt(char *out, char *in, int l)
{
	uint8_t len;

	len = (uint8_t) l;
	if (len > MAXNAM) len=MAXNAM;

	if (strlen(in) < (size_t)len) len = (int)strlen(in);

        uint8_t *txt=(uint8_t *) in;
        if (!len)
	        return;
	switch (*txt) {
	case 0x01 ... 0x0f:
	        txt++;
	        len--;
	        break;
	case 0x10:
	        txt+=3;
                len-=3;
	        break;
	}
	while (len>0) {
	        switch(*txt) {
		case '\"':
		case 0x01 ... 0x1f:
		case 0x7f ... 0xa0:
		        len--;
			txt++;
			break;
		case 0x00:
		        len=1;
		default:
		        *(out++)=(char) *(txt++);
			len--;
			break;
		}
	}
}

void show_buf(uint8_t *buf, int length)
{
	fprintf(stderr,"\n");
	for (int i=0; i<length; i+=8){
		int j;
		for (j=0; j < 8 && j+i<length; j++)
			fprintf(stderr,"0x%02x ", int(buf[i+j]));
		for (int r=j; r<8; r++) 			
			fprintf(stderr,"     ");

		for (j=0; j < 8 && j+i<length; j++){
			switch(buf[i+j]){
			case '0'...'Z':
			case 'a'...'z':
				fprintf(stderr,"%c", buf[i+j]);
				break;
			default:
				fprintf(stderr,".");
			}
		}
		fprintf(stderr,"\n");
	}
}


DVB::~DVB() {
        delete [] lnbs;
	delete [] tps;
	delete [] chans;
	delete [] sats;
	if (no_open) return;
	osd.Clear();
	osd.Hide();
	close(fd_frontend);
	close(fd_demuxa);
	close(fd_demuxv);
	close(fd_demuxpcr);
	close(fd_demuxtt);
	close(fdvb);
	free(vtxdir);
}

void DVB::init(char *dvbname, char *siname, int adapt, int min) {
	struct dvb_frontend_info feinfo;

	int failed =0;

	minor = min;
	adapter = adapt;

	magazin[0].magn = MAX_MAG;
	for(int i=1; i<MAX_MAG; i++){
		magazin[i].magn = i;
		magazin[i].valid = 0;
	}


	for (int i=0; i<NK; i++)
	        num[i]=0;

	if (lnbs) delete [] lnbs;
	lnbs   =new Lnb[maxs[LNB]];

	if (tps) delete [] tps;
	tps    =new Transponder[maxs[TRANS]];

	if (chans) delete [] chans;
	chans  =new Channel[maxs[CHAN]];

	if (sats) delete [] sats;
	sats   =new Sat[maxs[SAT]];


	if (minor < 0) return;

	if (no_open) return;
        if (fd_frontend > 0) close(fd_frontend);
        if (fd_demuxa > 0) close(fd_demuxa);
        if (fd_demuxv > 0) close(fd_demuxv);
        if (fd_demuxpcr > 0) close(fd_demuxpcr);
        if (fd_demuxtt > 0) close(fd_demuxtt);
	char devname[80];

	set_vtxdir(VTXDIR);

	dvr_enabled = 0;

	sprintf(devname,FRONT_DEV,adapter,minor);
	fd_frontend=open(devname, O_RDWR);
	if (fd_frontend < 0) {
		cerr << "Could not open " << devname << endl;
		front_type=-1;
		perror(devname);
		fd_frontend = -1;
		failed = 1;
	}
	ioctl(fd_frontend, FE_GET_INFO, &feinfo);
	front_type=feinfo.type;

	sprintf(devname,DEMUX_DEV,adapter,minor);
	fd_demuxtt=open(devname, O_RDWR);
	if (fd_demuxtt < 0) {
		cerr << "Could not open " << devname << endl;
		perror(devname);
		fd_demuxtt = -1;
		failed = 1;
	}

	fd_demuxa=open(devname, O_RDWR);
	if (fd_demuxa < 0) {
		cerr << "Could not open " << devname << endl;
		perror(devname);
		fd_demuxa = -1;
		failed = 1;
	}

	fd_demuxpcr=open(devname, O_RDWR);
	if (fd_demuxpcr < 0) {
		cerr << "Could not open " << devname << endl;
		perror(devname);
		fd_demuxpcr = -1;
		failed = 1;
	}

	fd_demuxv=open(devname, O_RDWR);
	if (fd_demuxv < 0) {
		cerr << "Could not open " << devname << endl;
		perror(devname);
		fd_demuxv = -1;
		failed = 1;
	}

}

Transponder *DVB::find_tp(unsigned int tpid, unsigned int satid)
{
	Transponder *tp=NULL;
	int i;
	
	for (i=0; i<num[TRANS]; i++) {
	        if (tps[i].id==tpid && tps[i].satid==satid) {
		        tp=&tps[i];
			break;
		}
	}

	return tp;
}

Transponder *DVB::find_tp(Channel *chan)
{
	Transponder *tp=NULL;
	int i;

	for (i=0; i<num[TRANS]; i++)  
               if (tps[i].id==chan->tpid) {
		        tp=&tps[i];
			break;
	       }
	return tp;
}

Sat *DVB::find_sat(Transponder *tp)
{
	Sat *sat=NULL;
	int i;

	for (i=0; i<num[SAT]; i++)  
               if (sats[i].id==tp->satid) {
		        sat=&sats[i];
			break;
	       }
	return sat;
}

Sat *DVB::find_sat(Channel *chan)
{
	Sat *sat=NULL;
	int i;

	for (i=0; i<num[SAT]; i++)  
               if (sats[i].id==chan->satid) {
		        sat=&sats[i];
			break;
	       }
	return sat;
}

Lnb *DVB::find_lnb(Sat *sat)
{
	Lnb *lnb=NULL;
	int i;

	for (i=0; i<num[LNB]; i++)  
               if (lnbs[i].id==sat->lnbid) {
		        lnb=&lnbs[i];
			break;
	       }
	return lnb;
}

int DVB::SetTP(unsigned int tpid, unsigned int satid)
{
        int i;
	Transponder *tp=0;
	Sat *sat=0;
	Lnb *lnb=0;

	if (no_open) return -1;

	tp = find_tp(tpid, satid);

	if (!tp) {
		fprintf(stderr,"Transponder not found!\n");
	        return -1;
	}

	for (i=0; i<num[SAT]; i++) {
		if (sats[i].id==tp->satid) {
		        sat=&sats[i];
			break;
		}
	}

	if (!sat){
		fprintf(stderr,"Satellite not found!\n");
	        return -1;
	}

	for (i=0; i<num[LNB]; i++)  
               if (lnbs[i].id==sat->lnbid) {
		        lnb=&lnbs[i];
			break;
	       }

	if (!lnb){
		fprintf(stderr,"LNB not found!\n");
	        return -1;
	}


	switch (front_type) {
	case FE_QPSK:
	        if (tp->freq < lnb->slof) {
		        front_param.frequency = (tp->freq - lnb->lof1);
			tone = SEC_TONE_OFF;
		} else {
		        front_param.frequency = (tp->freq - lnb->lof2);
			tone = SEC_TONE_ON;
		}
		if (tp->pol) voltage = SEC_VOLTAGE_18;
		else voltage = SEC_VOLTAGE_13;
		set_diseqc_nb(lnb->diseqcnr);
		front_param.u.qpsk.symbol_rate = tp->srate;
		front_param.u.qpsk.fec_inner = (fe_code_rate_t)tp->fec;
		front_param.inversion = tp->inversion;

		transponder_srate = tp->srate;
		transponder_pol = tp->pol ? 'H':'V';
		break;
	case FE_QAM:
	        front_param.frequency = tp->freq;
		front_param.inversion = tp->inversion;
		front_param.u.qam.symbol_rate = tp->srate;
		front_param.u.qam.fec_inner = (fe_code_rate_t)tp->fec;
		front_param.u.qam.modulation=(fe_modulation_t) (tp->qam+1);

		transponder_srate = tp->srate;
	        break;

	case FE_OFDM:
	        front_param.frequency = tp->freq;
		front_param.inversion = tp->inversion;
		front_param.u.ofdm.bandwidth = (fe_bandwidth_t)tp->band;
		front_param.u.ofdm.code_rate_HP = (fe_code_rate_t)tp->hp_rate;
		front_param.u.ofdm.code_rate_LP = (fe_code_rate_t)tp->lp_rate;
		front_param.u.ofdm.constellation = (fe_modulation_t)tp->mod;
		front_param.u.ofdm.transmission_mode = 
			(fe_transmit_mode_t)tp->transmode;
		front_param.u.ofdm.guard_interval = (fe_guard_interval_t)tp->guard;
		front_param.u.ofdm.hierarchy_information = 
			(fe_hierarchy_t)tp->hierarchy;
		break;
	}
	transponder_freq = tp->freq;
        return 0;
}

static uint16_t get_pid(uint8_t *pid)
{
        uint16_t pp;

        pp = (pid[0] & 0x1f) << 8;
        pp |= pid[1] &0xff;
        return pp;
}


static uint32_t bcd32_trafo(uint8_t *buf){
        return ((buf[0] >> 4) & 0x0f) * 10000000UL +
		(buf[0] & 0x0f) * 1000000UL +
		((buf[1] >> 4) & 0x0f) * 100000UL  + 
		(buf[1] & 0x0f) * 10000UL +
		((buf[2] >> 4) & 0x0f) * 1000UL    + 
		(buf[2] & 0x0f) * 100UL +
		((buf[3] >> 4) & 0x0f) * 10UL      + 
		(buf[3] & 0x0f);	
}

int DVB::parse_descriptor(Channel *chan, uint8_t *buf, int length, int verb=0, Transponder *tp=NULL)
{
	int dlength;
	Transponder dtp;

	if (!length) return 0;
	dlength = buf[1];
	if (verb)
		cerr << "desc 0x" << int(buf[0]) << endl;
	switch(buf[0]){
	case 0x0a:
		if (!chan) break;
		if (dlength==4 && chan->last_apidn>=0 && chan->last_apidn < MAXAPIDS){
			memcpy(chan->apids_name+chan->last_apidn*4,buf+2, 3);
		} 

		break;

	case 0x09:
		uint16_t casys;
		uint16_t capid;

		if (verb)
			cerr << "ECM descriptor" << endl;
		
		if (!dlength) break;
		if (!chan) break;
		casys =(buf[2]<<8)|buf[3];
		capid = get_pid(buf+4);
/*
		if ((dlength>17 && casys==0x100) || casys == 0xb00){
			capid = get_pid(buf+19);
		}
*/
		chan->casystem = casys;
		chan->capid = capid;
		AddECM(chan, buf, dlength+2);
		if (chan->ecm_callback) 
			chan->ecm_callback(chan);
		break;

	case 0x40: //network name descriptor
	{
		uint8_t slen=0;
		char name[MAXNAM+1];

		if (verb)
			cerr << "network name descriptor" << endl;
		if (!chan) break;
		memset (name,0,MAXNAM+1);
		slen = buf[1];
	      
		if (slen > MAXNAM){
			memcpy(name,(char*)(buf+2), 
			       MAXNAM);
			memset(chan->net_name, 0, MAXNAM+1);
			dvb2txt(chan->net_name,name, MAXNAM);
		} else if (slen) {
			memcpy(name,(char*)(buf+2), 
			       int(slen));
			memset(chan->net_name, 0, MAXNAM+1);
			dvb2txt(chan->net_name,name, int(slen));
		}

		break;
	}
	
	case 0x41:/* service list */
		if (verb)
			cerr << "service list descriptor" << endl;
		break;

	case 0x42:/* stuffing */
		if (verb)
			cerr << "stuffing descriptor" << endl;
		break;

	case 0x43:/* satellite delivery system */
		if (!tp) tp = &dtp;
		tp->type = FE_QPSK;
		tp->freq = 10*bcd32_trafo(buf+2);
		tp->srate = 10*bcd32_trafo(buf+9);
		tp->fec = ftab[buf[12] & 0x07];
		tp->pol = (buf[8] >> 5) & 0x03;
		tp->orbpos =  ((buf[6] >> 4) & 0x0f) * 1000 +     
			(buf[6] & 0x0f) * 100 +
			((buf[7] >> 4) & 0x0f) * 10 + 
			(buf[7] & 0x0f);
		if (buf[8] >> 7) tp->orbpos = -tp->orbpos;

		if (verb){
			cerr << "Satellite delivery system descriptor " 
			     << tp->orbpos
			     << endl;
			cerr << *tp;			
		}
		break;

	case 0x44:/* cable delivery system */
		if (!tp) tp=&dtp;
		tp->type = FE_QAM;
		tp->freq = 100*bcd32_trafo(buf+2);
		tp->srate = 10*bcd32_trafo(buf+9);
		tp->fec = ftab[buf[12] & 0x07];
		if ((buf[8] & 0x0f)<= 5)
			tp->qam = qtab[buf[8] & 0x0f]; 
		else 
			tp->qam = QAM_AUTO;

		if (verb){
			cerr << "Cable delivery system descriptor" 
			     << endl;
			cerr << *tp;
		}
		break;

	case 0x45:/* VBI data */
		if (verb)
			cerr << "VBI data descriptor" << endl;
		break;

	case 0x46:/* VBI teletext */
		if (verb)
			cerr << "VBI teletext descriptor" << endl;
		break;

	case 0x47:/* bouquet name */
		if (verb)
			cerr << "bouquet name descriptor" << endl;
		break;

	case 0x48: /* service desc */
	{
		uint8_t slen=0;
		char name[MAXNAM+1];
		uint8_t *mbuf = buf;

		if (verb)
			cerr << "service descriptor" << endl;
		if (!chan) break;
		memset (name,0,MAXNAM+1);
		mbuf += 3;
		slen = *mbuf;  /* provider name */
		mbuf++;
		if (slen > MAXNAM){
			memcpy(name, mbuf, MAXNAM);
			memset(chan->prov_name, 0, MAXNAM+1);
			dvb2txt(chan->prov_name,name, MAXNAM);
		} else if (slen) {
			memcpy(name, mbuf, int(slen));
			memset(chan->prov_name, 0, MAXNAM+1);
			dvb2txt(chan->prov_name,name, int(slen));
		}

		mbuf += slen;  /* service name */
		slen = *mbuf;
		mbuf++;
		if (!slen) break;
		memset (name,0,MAXNAM+1);
		if (slen > MAXNAM){
			memcpy(name,mbuf, MAXNAM);
			memset(chan->name, 0, MAXNAM+1);
			dvb2txt(chan->name,name, MAXNAM);
		} else if(slen>0){
			memcpy(name, mbuf, int(slen));
			memset(chan->name, 0, MAXNAM+1);
			dvb2txt(chan->name,name, int(slen));
		}
		if (verb)
			cerr << chan->name << endl; 

	}
	break;

	case 0x49:/* country availability */
		if (verb)
			cerr << "country availability descriptor" << endl;
		break;

	case 0x4a: /*linkage*/
		if (verb)
			cerr << "linkage descriptor "<< int(buf[8]) << endl;
		switch (buf[8]){
		case 0xb0: /* Multifeed */
		{	
			uint16_t ts_id, sid;
			ts_id = (buf[2] << 8)|buf[3];
			sid   = (buf[6] << 8)|buf[7];
			if (verb)
				cerr << HEX(2)<< ts_id << " " << HEX(2) << sid << endl;
			chan = NULL;
			for (int i = 0; i < num[CHAN]; i++){
				if (chans[i].pnr == sid){
					chan = &chans[i];      
					break;
				}
			       
			}

			uint8_t slen=0;
			char name[MAXNAM+1];
			uint8_t *mbuf = buf;
				
			memset (name,0,MAXNAM+1);
			mbuf += 9;
			slen = dlength-9;  /* multifeed name */
			if (slen > MAXNAM){
				memcpy(name, mbuf, MAXNAM);
				if (chan){
					memset(chan->name, 0, MAXNAM+1);
					dvb2txt(chan->name,name, MAXNAM);
				}
			} else if (slen) {
				memcpy(name, mbuf, int(slen));
				if (chan){
					memset(chan->name, 0, MAXNAM+1);
					dvb2txt(chan->name,name, MAXNAM);
				}
			}
			if (verb){
				if (chan)
					cerr << chan->name << endl; 
				else 
					cerr << name << endl;
			}
		}
		break;
		}
	
	case 0x4b:/* NVOD reference */
		if (verb)
			cerr << "NVOD reference descriptor" << endl;
		break;

	case 0x4c:/* time shifted service */
		if (verb)
			cerr << "time shifted service descriptor" << endl;
		break;

	case 0x4d: /* short event */
		if (verb)
			cerr << "short event descriptor" << endl;
		break;

	case 0x4e: /* extended event */
		if (verb)
			cerr << "extended event descriptor" << endl;
		break;

	case 0x4f: /* time shifted event */
		if (verb)
			cerr << "time shifted event descriptor" << endl;
		break;

	case 0x50: /* component event */
		if (verb)
			cerr << "component event descriptor" << endl;
		break;

	case 0x51: /* mosaic */
		if (verb)
			cerr << "mosaic descriptor" << endl;
		break;

	case 0x52: /* stream identufier */
		if (verb)
			cerr << "stream identifier descriptor" << endl;
		break;

	case 0x53: /* CA identifier */
		if (verb)
			cerr << "CA identifier descriptor" << endl;
		break;

	case 0x54: /* content */
		if (verb)
			cerr << "content descriptor" << endl;
		break;

	case 0x55: /*  parental rating*/
		if (verb)
			cerr << "parental rating descriptor" << endl;
		break;

	case 0x56: /* teletext */
		if (verb)
			cerr << "teletext descriptor" << endl;
		if (!chan) break;
		if (chan->tmppid != NOPID){
			chan->ttpid = chan->tmppid;
		}
		break;
		
	case 0x57: /* telephone */
		if (verb)
			cerr << "telephone descriptor" << endl;
		break;

	case 0x58: /*  local time offset */
		if (verb)
			cerr << "local time offset descriptor" << endl;
		break;

	case 0x59: /* subtitle */
		if (verb)
			cerr << "subtitle descriptor" << endl;
		if (!chan) break;
		if (chan->tmppid != NOPID){
			chan->subpid = chan->tmppid;
		}
		break;
		
	case 0x5a: /*  terrestrial delivery system */
		if (!tp) tp=&dtp;
		tp->type = FE_OFDM;
		if (verb){
			cerr << "Terrestrial delivery system descriptor" 
			     << endl;
			cerr << *tp;
		}
		break;

	case 0x5b: /*  multilingual network name */
		if (verb)
			cerr << "multilingual network name descriptor" << endl;
		break;

	case 0x5c: /*  multilingual bouquet name */
		if (verb)
			cerr << "multilingual bouquet name descriptor" << endl;
		break;

	case 0x5d: /*  multilingual service name */
		if (verb)
			cerr << "multilingual service name descriptor" << endl;
		break;

	case 0x5e: /*  multilingual component */
		if (verb)
			cerr << "multilingual component descriptor" << endl;
		break;

	case 0x5f: /* private data specifier  */
		if (verb)
			cerr << "private data specifier descriptor" << endl;
		break;

	case 0x60: /* service move */
		if (verb)
			cerr << "service move descriptor" << endl;
		break;

	case 0x61: /* short smoothing buffer */
		if (verb)
			cerr << "short smoothing buffer descriptor" << endl;
		break;

	case 0x62: /* frequency list */
		if (verb)
			cerr << "frequency list descriptor" << endl;
		break;

	case 0x63: /* partial transport stream */
		if (verb)
			cerr << "partial transport stream descriptor" << endl;
		break;

	case 0x64: /* data broadcast */
		if (verb)
			cerr << "data broadcast descriptor" << endl;
		break;

	case 0x65: /* CA system */
		if (verb)
			cerr << "CA system descriptor" << endl;
		break;

	case 0x66: /* data broadcast id */
		if (verb)
			cerr << "data broadcast id descriptor" << endl;
		break;

	case 0x67: /* transport stream offset */
		if (verb)
			cerr << "transport stream descriptor" << endl;
		break;

	case 0x68: /* DSNG */
		if (verb)
			cerr << "DSNG descriptor" << endl;
		break;

	case 0x69: /* PDC */
		if (verb)
			cerr << "PDC descriptor" << endl;
		break;

	case 0x6a: /* AC3 */
		if (verb)
			cerr << "AC3 descriptor" << endl;
		if (!chan) break;
		if (chan->tmppid != NOPID){
			chan->ac3pid = chan->tmppid;
		}
		break;

	case 0x6b: /* ancillary data */
		if (verb)
			cerr << "ancillary data descriptor" << endl;
		break;

	case 0x6c: /* cell list */
		if (verb)
			cerr << "cell list descriptor" << endl;
		break;

	case 0x6d: /* cell frequency link */
		if (verb)
			cerr << "cell frequency link descriptor" << endl;
		break;

	case 0x6e: /* announcement support */
		if (verb)
			cerr << "announcement support descriptor" << endl;
		break;

	default:
		//show_buf(buf,dlength);
		break;
	}    
		
	int c=0;
	c += dlength+2;
	if ( c < length) 
		c += parse_descriptor(chan, buf+c, length-c, verb);
	
	if (c<length){
		cerr << "Hmm error in descriptor parsing" << endl;
	}
	
	if (chan) chan->tmppid = NOPID;
	
	return length;
}

int DVB::parse_pmt(Channel *chan, uint8_t *buf)
{
	int slen, ilen, eslen, c;
	uint16_t epid;

	if (buf[0] != 0x02) return -1;
	slen = (((buf[1]&0x03) << 8) | buf[2]) +3;
	ilen = ((buf[10]&0x03)  <<8) | buf[11];
	chan->pcrpid =  get_pid(buf+8);
	c = 12;

	if (ilen) c+=parse_descriptor(chan, buf+c, ilen);
	if (c-12< ilen){
		cerr << "Hmm error in descriptor parsing" << endl;
	}

//	show_buf(buf,slen);
	while (c < slen-4){
		
		eslen = ((buf[c+3]&0x03)  <<8) | buf[c+4];
		epid = get_pid(buf+c+1);

		switch (buf[c]) {
		case 1: 
		case 2: 
			if (chan->vpid == NOPID)
				chan->vpid = epid; 
			break;
		case 3: 
		case 4: 
		{
			int afound = 0;
			uint16_t apid = epid;
			chan->last_apidn = -1;

			if (chan->apidnum>=MAXAPIDS) {
				cerr << "Need more apids\n";
				break;
			}
			
			for (int i = 0; 
			     i < chan->apidnum; i++){
				if ( apid == chan->apids[i]){
					afound = 1;
					chan->last_apidn = i;
					break;
				}
			}
			if (! afound){
				chan->apids[chan->apidnum++] = apid;
				chan->last_apidn = chan->apidnum-1;
			} 
			break; 
		}
		case 6: 
			chan->tmppid = epid;
			break;
/*
		case 5:
			cerr << "private service 0x" << hex << int(buf[c]) 
			     << " for service id 0x" << chan->pnr 
			     << " with pid 0x"<< epid << dec<< endl; 
			break;
		default:
			cerr << "other service 0x" << hex << int(buf[c]) 
			     << " for service id 0x" << chan->pnr 
			     << " with pid 0x"<< epid << dec<< endl; 
			break;
*/	
	}
		c += 5;
		if (eslen) c += parse_descriptor(chan, buf+c, eslen);
	}
	
	return 0;

}


int DVB::parse_pat(Channel *chan, uint8_t *buf)
{
	int slen, i, nprog;
	int found = 0;
	uint8_t *prog;
	uint16_t prog_pid = 0, pnr = 0;

	slen = (((buf[1]&0x03) << 8) | buf[2]);
	nprog = (slen - 9) / 4;
	prog = buf+ 8;
	for (i = 0; i < nprog; i++){
		pnr = (prog[0] << 8)|prog[1];
		if ( chan->pnr == pnr ){
			prog_pid = get_pid(prog+2);
			found = 1;
			break;
		} 
		prog += 4;
	} 

	if (found) found = prog_pid;
	
	return found;
}


/*
void DVB::parse_nit(Transponder *tp, uint8_t *buf)
{
	int slen, i, nprog;
	int found = 0;

	slen = (((buf[1]&0x0f) << 8) | buf[2]);
	
	

}

int DVB::check_nit(Transponder *tp)
{
	int found = 0;
	uint16_t prog_pid = 0;

	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	
	if (no_open) return -1;

	time_t count = time(0)+2;
	while (sec<=msec && !found && count > time(0)) {
		if (GetSection(buf, 0x10, 0x40, sec, msec)>0){
			sec++;
			found = parse_nit(tp, buf);
		}
	}
	if (!found) return -1;
	return 0;
}

*/

void DVB::check_all_pids()
{
	if (no_open) return;

	for (int i = 0; i < num[CHAN]; i++){
		cerr << "checking " << chans[i].name << endl;
		SetChannel(i); 
	}
}

int DVB::check_pids(Channel *chan)
{
	int found = 0;
	int oldnum;
	uint16_t prog_pid = 0;

	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	
	if (no_open) return -1;

	oldnum=chan->apidnum;
	time_t count = time(0)+4;
	while (sec<=msec && !found && count > time(0)) {
		if (GetSection(buf, 0, 0, sec, msec)>0 && !buf[0]){
			sec++;
			found = parse_pat(chan,buf);
		}
	}
	if (!found) return -1;
	prog_pid = found;
	chan->apidnum = 0;
	sec = 0;
	msec = 0;
	
	count = time(0)+4;
	while (sec<=msec && count > time(0)) {
		if (GetSection(buf,prog_pid, 2, sec, msec)>0){
			sec++;
			parse_pmt(chan, buf);
			if(count < time(0)) break;
		}
	}

	if (!chan->apidnum)
		chan->apidnum=oldnum;
	chan->checked = 1;
	return 0;
}

int DVB::get_all_progs(uint16_t *progbuf, uint16_t *pnrbuf, int length)
{
	int nfound = 0, oldone = 0, j;
	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	uint16_t prog_pid = 0, pnr = 0;
	int slen;
	uint8_t *prog;

	if (no_open) return -1;

	time_t count = time(0)+4;
	while (sec<=msec && count > time(0)) {
		if (GetSection(buf, 0, 0, sec, msec)>0 && !buf[0]){
			sec++;
			slen = (((buf[1]&0x0f) << 8) | buf[2]);
			slen -= 11;
			prog = buf+ 8;
			while (slen>0){ 
				pnr = (prog[0] << 8)|prog[1];
				prog_pid = get_pid(prog+2);
				
				oldone = 0;
				for(j=0; j<nfound; j++)
					if (pnr == pnrbuf[j]) oldone = 1;
					
				if ( !oldone && nfound < length && pnr ){
					pnrbuf[nfound]=pnr;
					progbuf[nfound]=prog_pid;
					nfound++;
				}
				slen -= 4;
				prog += 4;
			}
		}
	}
	return nfound;
}

int DVB::get_pids(uint16_t prog_pid, uint16_t *vpid, uint16_t *apids, 
		  uint16_t *ttpid, uint8_t *apids_name)
{
	int found = 0;
	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	int apidnum = 0;
	Channel chan;

	
	if (no_open) return -1;

	time_t count = time(0)+3;

	sec = 0;
	msec = 0;
	while (sec<=msec && count > time(0)) {
		if (GetSection(buf,prog_pid, 2, sec, msec)>0){
			sec++;
			parse_pmt(&chan,buf);
			if(count < time(0)) break;
		}
	}

	*ttpid = chan.ttpid;
	*vpid = chan.vpid;
	apidnum = chan.apidnum;
	if (apidnum && apidnum <= MAXAPIDS){
		memcpy(apids, chan.apids, apidnum * sizeof (uint16_t));
		if (apids_name) memcpy(apids_name, chan.apids_name, apidnum * sizeof (uint8_t)*4);
	}	
	return found+apidnum;
}

uint16_t DVB::find_pnr(uint16_t svpid, uint16_t sapid)
{
	int nfound = 0;
	int pfound = 0;
	uint16_t progs[100];
	uint16_t pnrs[100];
	uint16_t vpid;
	uint16_t ttpid;
	uint16_t apids[MAXAPIDS];

	if (no_open) return 0;
		
	nfound = get_all_progs(progs, pnrs, 100);
	for(int i=0; i < nfound; i++){
		if ( (pfound = get_pids(progs[i], &vpid, apids, &ttpid)) ){
			if(svpid != NOPID && vpid == svpid ) return pnrs[i];
			else if ( svpid == NOPID ){
				for (int j=0; j<pfound; j++){
					if (apids[j] == sapid) return pnrs[i];
				}
			}
		}
	}
	return 0;
}



void DVB::scan_sdt(Channel *chan)
{
	int slen, ilen, c;
	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	uint16_t pnr;
	uint8_t ca;
	uint16_t tsid;
	Transponder *tp;

	if (no_open) return;

	tp = find_tp(chan);
	time_t count = time(0)+15;
	while (sec<=msec && count > time(0)) {
		if (GetSection(buf, 0x11, 0x42, sec, msec)>0){
			sec++;
			slen = (((buf[1]&0x0f) << 8) | buf[2]) +3;
			tsid = (buf[3]<<8)|buf[4+1];
			if (tp) tp->tsid = tsid;
			c = 11;
			while (c < slen-4){
				pnr = (buf[c]<<8)|buf[c+1];
				chan->has_eit = -1;
				chan->pres_follow = -1;
				if (buf[c+2] & 0x02) chan->has_eit = 0;
				if (buf[c+2] & 0x01) chan->pres_follow = 0;
				c+=3;
				ca=(buf[c]&0x10)>>4;
				ilen=((buf[c]&0x0f)<<8)|buf[c+1];
				c+=2;
				if ( chan->pnr == pnr && ilen ){
					chan->type=ca;
					c+=parse_descriptor(chan, &buf[c], ilen);
				} else c+=ilen;
			}
		}
	}
}

int DVB::scan_sdts(int *chs, int n)
{
	int slen, ilen, c;
	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	uint16_t pnr;
	uint8_t ca;
	int *check;
	uint16_t tsid;
	Transponder *tp;

	if (n>MAX_TRANS_CHAN || n <0) return -1;
	if (no_open) return -1;
	check = new int[n];

	tp = find_tp(&chans[chs[0]]);
	for (int i=0; i<n; i++) check[i]=0;

	time_t count = time(0)+15;
	while (sec<=msec && count > time(0)) {
		if (GetSection(buf, 0x11, 0x42, sec, msec)>0){
			sec++;
			slen = (((buf[1]&0x0f) << 8) | buf[2]);
			tsid = (buf[3]<<8)|buf[4+1];
			if (tp) tp->tsid = tsid;
			c = 11;
			while (c < slen-4){
				pnr = (buf[c]<<8)|buf[c+1];
				c+=3;
				ca=(buf[c]&0x10)>>4;
				ilen=((buf[c]&0x0f)<<8)|buf[c+1];
				c+=2;
				int ce=0;
				for (int cl=0; cl < n; cl++){ 
					if (chans[chs[cl]].pnr == pnr ){
						ce=parse_descriptor(&chans[chs[cl]],buf+c, ilen);
						check[cl]=1;
						chans[chs[cl]].type=ca;
						continue;
					}
				}
				if (ce < ilen)
					cerr << "Error in descriptor parsing" << endl;
				c+=ilen;
			}
		} 
	}
	int found=0;
	for (int i=0; i < n; i++) found+=check[i];
	delete [] check;
	return found;
}


int eit_cb(uint8_t *buf, int l, int pnr, int c_n, uint8_t *t) 
{
	cout << "Type: " << c_n << "  PNR:" << pnr  << "  Time: " 
	     << hex << int(t[2])
	     << ":" << int(t[3])
	     << "." << int(t[4]) << dec << endl;
	for (int i=0; i < l/16+1; i++){
		cout << "0x" << HEX(4) << i << dec << "  ";
		for (int j=0; j < 16; j++){
			if (j+i*16 < l)
				cout << HEX(2) << int(buf[j+i*16]) << dec <<
					" ";
			else cout << "   ";
		}
		for (int j=0; j < 16 && j+i*16 < l; j++){
			uint8_t c = (char)buf[j+i*16];
			switch ((int)c) {
			case 0x01 ... 0x1f:
			case 0x7f ... 0xa0:
			case 0x00:
				cout << "." ;
				break;
			default:
				cout << char(c) ;
				break;
			}
		}
		cout << endl;
	}
	cout << endl;
	cout << endl;
	if (c_n && l>10) return 1;
	else return 0;
}


void DVB::scan_pf_eit(Channel *chan, 
		      int (*callback)(uint8_t *data, int l, int pnr, int c_n, 
				      uint8_t *t) 
		      = eit_cb)
{
	int slen, ilen, c, t;
	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	uint16_t pnr = NOID;
	int8_t ver;
	int c_n;
	int done=0;

	if (no_open) return;

	time_t count = time(0)+5;
	while ( !done && count > time(0)) {
		if (GetSection(buf, 0x12, 0x4e, sec, msec)>0){
			sec++;
			c = 1;
			slen = (((buf[c]&0x0f) << 8) | buf[c+1]) +3;
			c += 2;
			pnr = (buf[c]<<8)|buf[c+1];
			c += 2;
			ver = buf[c] & 0x3E;
			c_n = buf[c] & 0x01;
			c += 8;
			if (pnr == chan->pnr){
				while (c < slen-4){
					t = c+3;
					c += 10;
					ilen=((buf[c]&0x0f)<<8)|buf[c+1];
					c+=2;
					done = callback(buf+c, ilen, pnr, 
							c_n, buf+t);
					c+=ilen;
				}
			}
		}				
	}
}

void DVB::scan_pf_eit(int chnr,
		      int (*callback)(uint8_t *data, int l, int pnr, int c_n, 
				      uint8_t *t)  
		      = eit_cb) 
{
	if (no_open) return;

	if (chnr>=num[CHAN] || chnr < 0) return;
	scan_pf_eit(&chans[chnr], callback);
}

void DVB::scan_pf_eit(int chnr) 
{
	if (no_open) return;
	
	if (chnr>=num[CHAN] || chnr < 0) return;
	scan_pf_eit(&chans[chnr]);
}


int DVB::SetChannel(Channel *chan,  char* apref, uint16_t *apidp, 
		    uint16_t *vpidp) 
{
	int i;
	int scan = 0;
	uint16_t apid=0, vpid=0;
	
	if (no_open) return -1;

	if (chan->pnr == NOPID && (chan->vpid != NOPID || 
				   chan->apids[0] != NOPID))
		chan->pnr = find_pnr(chan->vpid, chan->apids[0]);
	
	int c=0;
	if (chan->pnr)
		if (chan->vpid == NOPID){
			check_pids(chan);
			scan=1;
			while (chan->apidnum == 0 && c<10) {
				check_pids(chan);
				c++;
			}
		}
	
	vpid = chan->vpid;
	if (chan->apidnum){
		if (apref){
			int found = 0;
			for (i=0; i < chan->apidnum; i++){
				if (!strncasecmp(chan->apids_name+4*i,
						 apref,3)){
					found=1;
					break;
				}
			}
			if (found) apid = chan->apids[i];
			else apid = chan->apids[0];
		} else apid = chan->apids[0];
	}

	if (vpid != NOPID) set_vpid(vpid);	
	set_apid(apid);
	set_pcrpid(chan->pcrpid);
	if (chan->ttpid != NOPID) set_ttpid(chan->ttpid);
	
	if (scan)
		scan_sdt(chan);

	if (fdvb >= 0){
		struct tm *lt;
		time_t t;
		
		
		t = time(0);
		lt = localtime(&t);
		
		ostringstream fstr;
		osd.Clear();
		
		fstr << setw(2) << setfill('0') << lt->tm_hour 
		     << ":" << setw(2) << setfill('0') 
		     << lt->tm_min << "  ";
		 
		if (chan->name[0])
			fstr << chan->name;
		else if (chan->prov_name[0])
			fstr << chan->prov_name;
		else fstr << "Channel " << dec << chan->id;
		fstr <<  ends;
		
		osd.Text(0, 0, 0, 1, fstr.str().data());
		osd.Show();
	}
	if (vpidp) *vpidp = vpid;
	if (apidp) *apidp = apid;
//	scan_pf_eit(chan);

	if (vpid==NOPID && apid == NOPID) return -2;
        return 0;

}

int DVB::SetChannel(int chnr, char* apref, uint16_t *apidp, 
		    uint16_t *vpidp, bool tune) 
{
	if (no_open) return -1;

	if (chnr>=num[CHAN] || chnr < 0)
	        return -1;

	if(tune) {
		get_front();
		if (SetTP(chans[chnr].tpid, chans[chnr].satid) < 0) return -1;
	}
	stop_apid();
	stop_vpid();
	stop_ttpid();
	stop_pcrpid();
	if(tune && (set_front()<0)) return -1;
       
	return SetChannel(&chans[chnr], apref, apidp, vpidp);
}



int DVB::SetChannel(uint16_t sid, uint16_t onid, 
		    uint16_t tpid, uint16_t satid)
{
        int chnr;
	Channel *chan=0;

	if (no_open) return -1;

	for (chnr=0; chnr<num[CHAN]; chnr++)  
	        if (chans[chnr].pnr==sid   && 
		    (onid==NOID  || chans[chnr].onid==onid) && 
		    (satid==NOID || chans[chnr].satid==satid)  && 
		    (tpid==NOPID  || chans[chnr].tpid==tpid)
		    ) {
		        chan=&chans[chnr];
			break;
		}
	if (!chan)
	        return -1;

	if (tpid==NOID)
 	        tpid=chan->tpid;
	if (onid==NOID)
 	        onid=chan->onid;
	if (satid==NOID)
 	        satid=chan->satid;

	get_front();
	if (SetTP(tpid, satid) < 0) return -1;
	if (set_front() < 0) return -1;

        set_vpid(chan->vpid);
        set_apid(chan->apids[0]);
	set_ttpid(chan->ttpid);
	set_pcrpid(chan->pcrpid);

	return chnr;
}

int DVB::GetChannel(int chnr, struct channel *) 
{
        int i;
	Channel *chan=0;
	Transponder *tp=0;
	Sat *sat=0;
	Lnb *lnb=0;

	if (chnr>=num[CHAN])
	        return -1;
	chan=&chans[chnr];
	
	tp = find_tp(chan);
	if (!tp)
	        return -1;

	sat = find_sat(tp);
	if (!sat)
	        return -1;

	for (i=0; i<num[LNB]; i++)  
               if (lnbs[i].id==sat->lnbid) {
		        lnb=&lnbs[i];
			break;
	       }
	if (!lnb || no_open)
	        return -1;
	if (set_front() < 0) return -1;

        return 0;

}

int DVB::AddLNB(int id, int t, uint l1, uint l2, uint sl,
		int dnr, int dis, int sw)
{
        if (num[LNB]>=maxs[LNB])
	        return -1;
	for (int i=0; i< num[LNB]; i++){
		if (lnbs[i].id == id && lnbs[i].diseqcnr == dnr){
			cerr << "Warning: LNB already defined:" << endl;
			cerr << "ID: " << id << "  DISEQCNR: " << dnr << endl;
			return -1;
		}
	}

	lnbs[num[LNB]].init(t, l1, l2, sl, dnr, dis, sw);
	lnbs[num[LNB]].id=id;
	num[LNB]++;
	return 0;
}



int DVB::AddTP(Transponder &tp)
{
        if (num[TRANS]>=maxs[TRANS])
	        return -1;

	if (tp.id == NOID){
		max_tpid++;
	        tp.id = max_tpid;
	} else if (tp.id > max_tpid) max_tpid = tp.id;

	for (int i=0; i<num[TRANS]; i++)
		if (tps[i].id==tp.id && tps[i].satid == tp.satid){
			cerr << "Warning: TP already defined:" 
			     << endl;
			cerr << "ID: " << hex << tp.id ;
			cerr << "  SATID: "<< hex  << tp.satid;
			cerr << endl;
			return i;
		} 
			
	tps[num[TRANS]]=tp;
	num[TRANS]++;
	return num[TRANS]-1;
}

void DVB::find_satid(Channel &chan)
{
	for (int i=num[TRANS]; i >= 0; i--){
		if (chan.tpid == tps[i].id){
			chan.satid = tps[i].satid;
			return;
		}
	}
}


int DVB::AddChannel(Channel &chan)
{
        int i;

        if (num[CHAN]>=maxs[CHAN])
	        return -1;
	
	if ( chan.satid == NOID)
		find_satid(chan);

        for (i=0; i<num[CHAN]; i++) {
		if (chan.pnr != NOPID && chan.pnr == chans[i].pnr && 
		    chan.satid == chans[i].satid && chan.tpid == chans[i].tpid) {
			return i;
		}
		if (chan.pnr == NOPID && chan.vpid == chans[i].vpid &&
		    chan.apids[0] == chans[i].apids[0] &&
		    chan.satid == chans[i].satid
		    && chan.tpid == chans[i].tpid) {
		        cerr << "Channel " << chan.name << " ("
			     << hex << chan.pnr << ") exists" << endl;
			return i;
		}
	}

	chan.id = num[CHAN];
	chans[num[CHAN]] = chan;
	num[CHAN]++;
	return chan.id;
}

int DVB::AddSat(Sat &sat)
{
        int i;

        if (num[SAT]>=maxs[SAT])
	        return -1;
	if (!sat.id)
	        sat.id=num[SAT];

        for (i=0; i<num[SAT]; i++) {
 	        if (sat.lnbid == sats[i].lnbid) {
		        cerr << "Sat exists\n";
			return i;
		}
	}
	sats[num[SAT]]=sat;
	num[SAT]++;
	return sat.id;
}

int DVB::AddSat(int id, unsigned int lnbid, char *name, uint fmin, uint fmax)
{
        int i,j;

	if (num[SAT]==maxs[SAT]) 
	        return -1;
	for (i=0; i<num[LNB]; i++) {
	  if (lnbs[i].id==lnbid) {
		  for (j=0; j<num[SAT]; j++) {
			  if (lnbid == sats[j].lnbid) {
				  cerr << "Sat exists\n";
				  return j;
			  }
		  }
		  sats[num[SAT]].id=id;
		  sats[num[SAT]].lnb=&lnbs[i];
		  sats[num[SAT]].lnbid=lnbs[i].id;
		  strncpy(sats[num[SAT]].name, name, maxname);
		  sats[num[SAT]].name[maxname]=0;
		  sats[num[SAT]].fmin=fmin;
		  sats[num[SAT]].fmax=fmax;
		  num[SAT]++;
		  return num[SAT]-1;
		}
	}
	return -1;

}


int DVB::search_in_TP(uint16_t tpid, uint16_t satid, int show, int verbose)
{
	int nfound = 0;
	int tries = 0;
	uint16_t progs[MAX_TRANS_CHAN];
	uint16_t pnrs[MAX_TRANS_CHAN];
	uint16_t vpid;
	uint16_t ttpid;
	uint16_t apids[MAXAPIDS];
	uint8_t apids_name[MAXAPIDS*4];
	int n=0;
        int chs[MAX_TRANS_CHAN];
	
	if (no_open) return -1;

	if (verbose < 2){
		get_front();
		if (verbose){
			cerr << "Setting Transponder 0x" << HEX(4) 
			     << tpid << "  ";
			for (int i = 0; i < num[TRANS]; i++){
				if (tps[i].id == tpid){
					cerr << dec << tps[i].freq/1000 
					     << (tps[i].pol ? "H":"V")
					     << " " << tps[i].srate/1000
					     << endl;
					break;
				}
			}
		}
		if (SetTP(tpid, satid) < 0) return -1;
		if (set_front() < 0) return -1;
	}

	if (verbose) cerr << " ... scanning ";
	while (!nfound && tries<3){
		nfound = get_all_progs(progs, pnrs, MAX_TRANS_CHAN);
		tries++;
	}
	if (verbose) cerr << dec << nfound;
	if (nfound > MAX_TRANS_CHAN){
		cerr << "found too many channels "<< nfound << endl;
		cerr << "resetting to"<< MAX_TRANS_CHAN << endl;
		nfound = MAX_TRANS_CHAN;
	}
	for(int i=0; i < nfound; i++){
 		Channel chan;
		int cn;
		int anum=0;

		vpid = NOPID;
		ttpid= NOPID;
		

		if ( show ){
			if (!(get_pids(progs[i], &vpid, apids, &ttpid, apids_name) && 
			      vpid != NOPID)) continue;
		} else anum = get_pids(progs[i], &vpid, apids, &ttpid, apids_name); 

		chan.pnr = pnrs[i];
		chan.satid = satid;
		chan.tpid = tpid;
		chan.vpid = vpid;
		chan.apidnum = anum;
		if (anum && anum < MAXAPIDS){ 
			memcpy(chan.apids, apids, anum*sizeof(uint16_t));
			memcpy(chan.apids_name, apids_name, anum*sizeof(uint8_t)*4);
		}
		chan.checked = 1; 
		chan.ttpid = ttpid;

		if (show){
			if (SetChannel(&chan) < 0) return 0;
			sleep(2);
		} 

		if ((cn=AddChannel(chan))==num[CHAN]-1){
			chs[n]=cn;
			n++;
			if (verbose) cerr << ".";
		}		
	}
	if (n > MAX_TRANS_CHAN){
		cerr << "counted too many channels "<< n << endl;
		cerr << "resetting to"<< MAX_TRANS_CHAN << endl;
		n = MAX_TRANS_CHAN;
	}
	if (!show){
		int count = 0;
		int res=0;
		while ((res=scan_sdts(chs,n))< n && res>0 && count < 2) count++;
	}
	if (verbose){
		cerr << endl;
		for (int i=0; i < n; i++){
			cerr << "Found " << chans[chs[i]];
		}
		cerr << dec;
	}
	return n;
}

int DVB::search_in_TP(Transponder &tp, int show, int verbose)
{
	if (no_open) return -1;
	return search_in_TP(tp.id, tp.satid, show, verbose);
}

void DVB::set_outtype(dvb_outtype type)
{
	outtype = type;
}

dvb_outtype DVB::get_outtype(void)
{
	return  outtype;
}

ostream &operator<<(ostream &stream, DVB &x) {
	int i,j,k,l;
	

	switch(x.outtype){
		
	default:
	case LIBDVB_OUT:
		
		for (i=0; i<x.num[LNB]; i++) {
			stream << x.lnbs[i];
		
			for (j=0; j<x.num[SAT]; j++)
				if (x.sats[j].lnbid==x.lnbs[i].id) {
					stream << x.sats[j];
				
					for (k=0; k<x.num[TRANS]; k++) 
						if (x.tps[k].satid==x.sats[j].id) {
							x.tps[k].type = x.front_type;
							stream << x.tps[k];
							
							for (l=0; l<x.num[CHAN]; l++) 
								if (x.chans[l].tpid==
								    x.tps[k].id && 
								    x.chans[l].satid == x.tps[k].satid )
									stream << x.chans[l];
						}
				}
		}
		break;

	case ZAP_OUT:
		for (i=0; i< x.num[CHAN]; i++){
			Channel *chan = &x.chans[i];
			Transponder *tp = x.find_tp(chan);
			Sat *sat = x.find_sat(chan);
			Lnb *lnb=NULL;

			if(chan->vpid == NOPID) continue;
			if(!strlen(chan->name)) continue;
			
			stream << chan->name << ":";
			switch (tp->type){
			case FE_QPSK:
				lnb = x.find_lnb(sat);
				if(!lnb) continue;
				
				stream << tp->freq/1000 << ":";
				if (tp->pol) stream << "h:";
				else stream << "v:";
				stream << lnb->diseqcnr << ":"
				       << tp->srate/1000 << ":"
				       << fec_name[tp->fec] << ":";
				break;
			
			case FE_QAM:
				stream << tp->freq << ":"
				       << inv_name[tp->inversion] << ":"
				       << tp->srate << ":"
				       << fec_name[tp->fec] << ":"
				       << qam_name[tp->mod] << ":";
				break;
				
			case FE_OFDM:
				stream << tp->freq << ":"
				       << inv_name[tp->inversion] << ":"
				       << bw_name[tp->band] << ":"
				       << fec_name[tp->hp_rate] << ":"
				       << fec_name[tp->lp_rate] << ":"
				       << qam_name[tp->mod] << ":"
				       << mode_name[tp->transmode] << ":"
				       << guard_name[tp->guard] << ":"
				       << hierarchy_name[tp->hierarchy] << ":";
				
				break;
			}
			stream << chan->vpid << ":" << chan->apids[0] << ":"
			       << chan->pnr << endl;
		}
		break;

	case VDR_OUT:
		for (i=0; i< x.num[CHAN]; i++){
			Channel *chan = &x.chans[i];
			Transponder *tp = x.find_tp(chan);
			Sat *sat = x.find_sat(chan);
			Lnb *lnb=NULL;

			if(chan->vpid == NOPID) continue;
			if(!strlen(chan->name)) continue;
			
			stream << chan->name << ":";


			switch (tp->type){
			case FE_QPSK:
				lnb = x.find_lnb(sat);
				if(!lnb) continue;
				
				stream << tp->freq/1000 << ":";
				if (tp->pol) stream << "h:";
				else stream << "v:";
				stream << "S" << hex << sat->id/16 << "." 
				       << hex << (sat->id & 0x0F) << "E:" << dec
				       << tp->srate/1000 << ":";
				break;
			
			case FE_QAM:
				stream << tp->freq/1000000 << ":M"
				       << vdr_qam_name[tp->mod] << ":C:"
				       << tp->srate/1000 << ":";
				break;
				
			case FE_OFDM:
				stream << tp->freq << "I"
				       << vdr_inv_name[tp->inversion] << "B"
				       << vdr_bw_name[tp->band] << "C"
				       << vdr_fec_name[tp->hp_rate] << "D"
				       << vdr_fec_name[tp->lp_rate] << "M"
				       << vdr_qam_name[tp->mod] << "T"
				       << vdr_mode_name[tp->transmode] << "G"
				       << vdr_guard_name[tp->guard] << "Y"
				       << vdr_hierarchy_name[tp->hierarchy] << ":T:27500:";
				
				break;
			}

			if (chan->pcrpid != NOPID && 
			    chan->pcrpid != chan->vpid)
				stream << chan->vpid << "+" << chan->pcrpid << ":";
			else
				stream << chan->vpid << ":";
			stream << chan->apids[0];
			
			for (l = 1; l < chan->apidnum; l++)
				stream << "," << chan->apids[l];
			if (chan->ac3pid != NOPID)
				stream << ";" << chan->ac3pid;
			stream << ":" << chan->ttpid;
			if(chan->capid != NOPID)
				stream << ":1";
			stream << ":" << chan->pnr << ":0:0:0" << endl;
		}
		break;

	case MYTH_OUT:
		break;
	
	}
	return stream;
}

int check_for_vdr_zap(int *f, istream &ins)
{
	string s;
	int c=0;
	

	while (!c && ! ins.eof()){
		c=0;
		getline (ins, s);
		for (unsigned int i = 0; i < s.length();i++)
			if (s[i] == ':') c++;
		if (c < 2 ) c= 0;
	}

//	cerr << c << endl;

	switch (c){
	case 12:
	{
		char *namebuf;
		int freq;
		sscanf(s.c_str(), "%a[^:]:%d ",&namebuf, &freq);
		free(namebuf);
		if (freq < 1000000){
			*f = DVB_VDR;
		} else {
			*f = DVB_ZAPT;
		}
		return 1;
	}			
	break;

	case 7:
		*f = DVB_ZAPC;
		return 1;
		break;

	case 8:
		*f = DVB_ZAPS;
		return 1;
		break;

	case 11:
		*f = DVB_ZAPT;
		return 1;
		break;
	default:
		return 0;

	}

}

int DVB::check_input_format(istream &ins)
{
	streampos pos = ins.tellg();
	int found = 0;
	char *test_keys[]={
		"LNB","TRANSPONDER","CHANNEL","SAT","<?xml", ":SAT", "SATCODX",
		NULL
	};
	enum { XML_START=4, NOKIA_START, SATCO_START};


	int f = -1;

	while(!found && !ins.eof()){
		int n;
		char keybuf[MAXNAM];
		ins.width(MAXNAM);
		ins >> keybuf;

		if ( strncmp(keybuf, test_keys[SATCO_START],7) )
			n=findkey(keybuf, test_keys);
		else n = SATCO_START;
		
		switch (n){
		case LNB:
		case TRANS:
		case CHAN:
		case SAT:
			found = 1;
			f = DVB_ORIG;
			break;
			
		case NOKIA_START:
			found = 1;
			f = DVB_NOKIA;
			break;
			
		case XML_START:
			found = 1;
			f = DVB_XML;
			break;

		case SATCO_START:
			found = 1;
			f = DVB_SATCO;
			break;
		default:
			ins.seekg(pos);
			found = check_for_vdr_zap(&f, ins);
			if (! found){
				cerr << "Error: " << keybuf 
				     << " is not a valid keyword at " 
				     << endl;
				exit(0);
			}
			
		}
	} 
	ins.seekg(pos);
	return f;
}

void DVB::read_original(istream &ins)
{
	char *names[] = {
		"LNB","TRANSPONDER","CHANNEL","SAT", NULL
	};

	while(!ins.eof()){
		char keybuf[MAXNAM];
		ins.width(MAXNAM);
		ins >> keybuf;
		int n=findkey(keybuf, names);
		if (n<0) {
			cerr << endl << "Error: " << keybuf 
			     << " is not a valid keyword at " 
			     << endl;
			exit(0);
		} else {
			if (num[n]< maxs[n]){
				switch (n){
				case LNB:
				{
					Lnb lnb;
					lnb.name[0]='\0';
					ins >> lnb;
					cerr << ".";
					AddLNB(lnb.id, lnb.type, lnb.lof1, 
						 lnb.lof2, lnb.slof, 
						 lnb.diseqcnr, lnb.diseqcid, 
						 lnb.swiid);
					front_type = lnb.type;
					break;
				}
				case TRANS:
				{
					Transponder tp;
					ins >> tp;
					AddTP(tp);
					break;
				}
				case CHAN:
				{
					Channel chan;
					ins >> chan;
					AddChannel(chan);
					break;
				}
				case SAT:
				{
					Sat sat;
					ins >> sat;
					AddSat(sat);
					break;
				}
				}
			} else {
				cerr << "not enough channels" << endl;
				break;
			}
		}
	} 
	cerr << " done" << endl;
	
}

istream &operator>>(istream &ins, DVB &x)
{
	int format = x.check_input_format(ins);

	switch(format){
	case DVB_ORIG:
		cerr << "Reading libdvb format" << endl; 
		x.read_original(ins);
		break;

	case DVB_NOKIA:
	{
		cerr << "Reading Nokia format" << endl; 
		nokiaconv cc(&x);
		cc.lnb_sat.n = 4;
		cc.lnb_sat.diseqc[0] = 0;
		cc.lnb_sat.diseqc[1] = 1;
		cc.lnb_sat.diseqc[2] = 2;
		cc.lnb_sat.diseqc[3] = 3;
		strncpy(cc.lnb_sat.sat_names[0],"Astra",5);
		cc.lnb_sat.satid[0]=0x0192;
		strncpy(cc.lnb_sat.sat_names[1],"HotBird",7);
		cc.lnb_sat.satid[1]=0x0130;
		strncpy(cc.lnb_sat.sat_names[2],"Sirius",6);
		cc.lnb_sat.satid[2]=0x0050;
		cerr << "Reading NOKIA format" << endl;
		cc.dvb->front_type = FE_QPSK;
		
		ins >> cc;
		break;
	}

	case DVB_XML:
	{
		cerr << "Reading XML format" << endl; 
		xmlconv cc(&x);

		cc.lnb_sat.n = 4;
		cc.lnb_sat.diseqc[0] = 0;
		cc.lnb_sat.diseqc[1] = 1;
		cc.lnb_sat.diseqc[2] = 2;
		cc.lnb_sat.diseqc[3] = 3;
		strncpy(cc.lnb_sat.sat_names[0],"Astra",6);
		cc.lnb_sat.satid[0]=0x0192;
		strncpy(cc.lnb_sat.sat_names[1],"HotBird",7);
		cc.lnb_sat.satid[1]=0x0130;
		strncpy(cc.lnb_sat.sat_names[2],"Sirius",6);
		cc.lnb_sat.satid[2]=0x0050;
		cerr << "Reading XML format" << endl;
		cc.dvb->front_type = FE_QPSK;
		
		ins >> cc;
		break;
	}

	case DVB_SATCO:
	{
		cerr << "Reading satco format" << endl; 
		satcoconv cc(&x);
		cc.nlnb = 0;
		cc.dvb->front_type = FE_QPSK;
		ins >> cc;		
		break;
	}	

	case DVB_VDR:
	{
		cerr << "Reading VDR format" << endl; 
		vdrconv cc(&x);
		ins >> cc;
		break;
	}

	case DVB_ZAPS:
	{
		cerr << "Reading ZAP Sat format" << endl; 
		zapconv cc(&x);
		cc.dvb->front_type = FE_QPSK;
		ins >> cc;
		break;
	}

	case DVB_ZAPC:
	{
		cerr << "Reading ZAP Cable format" << endl; 
		zapconv cc(&x);
		cc.dvb->front_type = FE_QAM;
		ins >> cc;
		break;
	}

	case DVB_ZAPT:
	{
		cerr << "Reading ZAP ter. format" << endl; 
		zapconv cc(&x);
		cc.dvb->front_type = FE_OFDM;
		ins >> cc;
		break;
	}

	default:
		cerr << "Unknown format. Can't open dvbrc. Exiting" << endl;
		exit(1);

	}
	return ins;
}




void hdump(uint8_t *buf, int n) 
{
	int i;
	
	for (i=0; i<n; i++)
		cerr << HEX(2) << (int) buf[i] << " ";
	cerr << endl;
}


void DVB::bar2(int x, int y, int w, int h, int val, int col1, int col2) 
{
	int sep=(w*val)>>16;
	
	if (fdvb >= 0) {
		osd.FillBlock(x, y, x+w-1-sep, y+h-1, col1);
		osd.FillBlock(x+w-1-sep, y, 515, y+h-1, col2);
	}
}



int DVB::SetFullFilter(uint16_t pid) 
{
	char devname[80];
	sprintf(devname,DEMUX_DEV,adapter,minor);
	struct dmx_pes_filter_params pesFilterParams; 

	int fd_section=open(devname, O_RDWR|O_NONBLOCK);
	
	if (fd_section < 0) return fd_section;
	
	pesFilterParams.input = DMX_IN_FRONTEND; 
	pesFilterParams.output = DMX_OUT_TS_TAP; 
	pesFilterParams.pes_type = DMX_PES_OTHER; 
	pesFilterParams.flags = DMX_IMMEDIATE_START;
	
	if (pid == NOPID )pesFilterParams.pid = DMX_FULL_TS_PID;
	else pesFilterParams.pid = pid;
	if (ioctl(fd_section, DMX_SET_PES_FILTER, &pesFilterParams) < 0) {
	        printf("Could not set PES filter\n"); 
		close(fd_section);
		return -1;
	}
	return fd_section;
}


uint16_t DVB::SetFilter(uint16_t pid, uint16_t section, uint16_t mode) 
{ 
	struct dmx_sct_filter_params secFilterParams;
	char devname[80];
	sprintf(devname,DEMUX_DEV,adapter,minor);

	int fd_section=open(devname, O_RDWR|mode);
	
	secFilterParams.pid=pid;
	memset(&secFilterParams.filter.filter, 0, DMX_FILTER_SIZE);
	memset(&secFilterParams.filter.mask, 0, DMX_FILTER_SIZE);
	memset(&secFilterParams.filter.mode, 0, DMX_FILTER_SIZE);
	secFilterParams.timeout = 0;
	secFilterParams.flags= secflags;
	
	secFilterParams.filter.filter[0]=(section>>8)&0xff;
	secFilterParams.filter.mask[0]=section&0xff;
	
	if (ioctl(fd_section, DMX_SET_FILTER, &secFilterParams) < 0)  
		return 0xffff;
	return fd_section;
} 

int DVB::SetFilter(uint16_t pid, uint8_t *filter, 
		   uint8_t *mask,
		   uint32_t timeout, uint32_t flags) 
{
	
	struct dmx_sct_filter_params secFilterParams;
	char devname[80];
	sprintf(devname,DEMUX_DEV,adapter,minor);
	
	int fd_section=open(devname, O_RDWR|flags);
	
	secFilterParams.pid=pid;
	memset(&secFilterParams.filter.filter, 0, DMX_FILTER_SIZE);
	memset(&secFilterParams.filter.mask, 0, DMX_FILTER_SIZE);
	memset(&secFilterParams.filter.mode, 0, DMX_FILTER_SIZE);
	secFilterParams.timeout = timeout;
	secFilterParams.flags= secflags;
	
	for (int i = 0; i < DMX_FILTER_SIZE; i++){
		secFilterParams.filter.filter[i]=filter[i];
		secFilterParams.filter.mask[i]=mask[i];
	}
	if (ioctl(fd_section, DMX_SET_FILTER, &secFilterParams) < 0)  
		return 0xffff;
	return fd_section;
}


int DVB::CloseFilter(int h) 
{
	if (no_open) return -1;

	close(h);
	return 0;
}


int DVB::GetSection(uint8_t *buf, ushort PID, uint8_t sec,
		    uint8_t secnum, uint8_t &msecnum) 
{
	int seclen=0;
	uint16_t handle, pid;
	uint8_t section, sectionnum=0xff, maxsec=0;
	struct pollfd pfd;
	int loopc = 0;
	if (no_open) return -1;

	if ((handle=SetFilter(PID, (sec<<8)|0x00ff, 0))==0xffff)
		return -1;
	do {
		seclen=0;
		pfd.fd = handle;
		pfd.events=POLLIN;
		if (poll(&pfd, 1, 2000)==0) {
			break;
		}
		loopc++;
		
		pid = PID;
		read(handle, buf, MAXSECSIZE);
		seclen = 0;
		seclen |= ((buf[1] & 0x0F) << 8); 
		seclen |= (buf[2] & 0xFF);
		seclen+=3;
		section=buf[0];
		sectionnum=buf[6];
		maxsec=buf[7];
	} while ( loopc < maxsec*2 && 
		 (section != sec || pid != PID || sectionnum != secnum));
	msecnum=maxsec;
	CloseFilter(handle);
	return seclen;
}

int DVB::GetSection(uint8_t *buf, 
		    uint16_t PID, uint8_t *filter, uint8_t *mask,
		    uint8_t secnum, uint8_t &msecnum) 
{
	int seclen=0;
	uint16_t handle, pid;
	uint8_t section, sectionnum=0xff, maxsec=0;
	struct pollfd pfd;
	int loopc = 0;

	if (no_open) return -1;

	if ((handle=SetFilter(PID, filter, mask, 0, 0))==0xffff)
		return -1;
	do {
		seclen=0;
		pfd.fd=handle;
		pfd.events=POLLIN;
		if (poll(&pfd, 1, 20000)==0)
			break;
		loopc++;
		pid = PID;
		read(handle, buf, MAXSECSIZE);
		seclen = 0;
		seclen |= ((buf[1] & 0x0F) << 8); 
		seclen |= (buf[2] & 0xFF);
		seclen+=3;
		section=buf[0];
		sectionnum=buf[6];
		maxsec=buf[7];
	} while (loopc < maxsec*2 && ((section&mask[0]!=filter[0]) ||
		 pid!=PID || sectionnum!=secnum));
	msecnum=maxsec;
	CloseFilter(handle);
	return seclen;
}

int DVB::GetSection(uint8_t *buf, 
		    uint16_t PID, uint8_t TID, uint16_t TIDExt, 
		    uint16_t FilterTIDExt, 
		    uint8_t secnum, uint8_t &msecnum) 
{
	uint8_t filter[16], mask[16];
	if (no_open) return -1;

	memset(filter, 0, 16);
	memset(mask, 0, 16);
	
	filter[0]=TID;
	mask[0]=0xff;
	if (TIDExt!=0xffff) {
		filter[1]=(TIDExt>>8);
		filter[2]=TIDExt&0xff;
		mask[1]=(FilterTIDExt>>8);
		mask[2]=FilterTIDExt&0xff;
		}
	
	return GetSection(buf, PID, filter, mask, secnum, msecnum);
}		

void DVB::get_front(void) 
{
	if (no_open) return;

	set_vpid(0);
	set_apid(0);
	set_ttpid(0);
	set_pcrpid(0);
	voltage = SEC_VOLTAGE_13; 
	tone = SEC_TONE_OFF;
}	        
  

int DVB::check_frontend()
{
	return chck_frontend(fd_frontend, &festat);
}


int DVB::tune_it(struct dvb_frontend_parameters *front_param)
{
	int chk=0;

	if (no_open) return -1;
	lastclock = times(&ts);
	if (ioctl(fd_frontend, FE_SET_FRONTEND, front_param) <0){
		perror("setfront front");
		return -1;
	}
	if (showtime)
		cerr << "set frontend time: "
		     << double(times(&ts)-lastclock)/
			double(sysconf(_SC_CLK_TCK))
		     << "s" << endl;

	lastclock = times(&ts);
	chk=check_frontend();
	if (showtime)
		cerr << "check frontend time: "
		     << double(times(&ts)-lastclock)/
			double(sysconf(_SC_CLK_TCK))
		     << "s" << endl;
        

	if (!chk){
		chk = check_frontend();
		if (!chk){
			cerr << "Tuning failed" << endl;
			return -1;
		}
	}

	return 0;
}


int DVB::set_front(void) 
{
	if (no_open) return -1;

	set_vpid(0);  
	set_apid(0);
	set_pcrpid(0);
	set_ttpid(0);

	if (front_type==FE_QPSK) set_diseqc();
	usleep(10000);

	int c=0;
	int ret=0;
	while (c<1 && (ret=tune_it(&front_param))<0){
		c++;
		usleep(10000);
		if (front_type==FE_QPSK) toggle_diseqc();
	}
	return ret;
}	        

void DVB::set_diseqc()
{
	if (ioctl(fd_frontend, FE_SET_TONE, SEC_TONE_OFF) <0)
		perror("FE_SET_TONE failed");

	if (ioctl(fd_frontend, FE_SET_VOLTAGE, voltage) <0)
		perror("FE_SET_VOLTAGE failed");
	usleep(15000);
	if(ioctl(fd_frontend, FE_DISEQC_SEND_MASTER_CMD, &dcmd) < 0)
		perror("FE_DISEQC_SEND_MASTER_CMD failed");
	usleep(15000);
	if(ioctl(fd_frontend, FE_DISEQC_SEND_BURST, burst) <0)
		perror("FE_DISEQC_SEND_BURST failed");
	usleep(15000);
	if(ioctl(fd_frontend, FE_SET_TONE, tone) < 0)
		perror("FE_SET_TONE failed");
	usleep(15000);
}

void DVB::toggle_diseqc()
{
	uint8_t old = dcmd.msg[3];
	dcmd.msg[3] ^= (old & 0x0C);  
	set_diseqc();
	dcmd.msg[3] = old;
	set_diseqc();
}

void DVB::set_diseqc_nb(int nr) 
{
	if (no_open) return;

	dcmd.msg[0]=0xe0;
	dcmd.msg[1]=0x10;
	dcmd.msg[2]=0x38;
	dcmd.msg[3]=0xF0 | 
		((nr * 4) & 0x0F) | 
		((tone == SEC_TONE_ON) ? 1 : 0) |
		((voltage==SEC_VOLTAGE_18) ? 2 : 0);
	dcmd.msg[4]=0x00;
	dcmd.msg[5]=0x00;
	dcmd.msg_len=4;
	burst=(nr&1) ? SEC_MINI_B : SEC_MINI_A;
}



void DVB::set_ttpid(ushort ttpid) 
{  
	if (no_open) return;

        if(set_ttpid_fd(ttpid, fd_demuxtt) < 0) {
	        printf("PID=%04x\n", ttpid);
		perror("set_ttpid");
	}
}

void DVB::stop_ttpid()
{
	stop_pid_fd(fd_demuxtt);
}



void DVB::set_vpid(ushort vpid) 
{  
	if (no_open) return;

        if(set_vpid_fd(vpid, fd_demuxv) < 0) perror("set_vpid");
}

void DVB::stop_vpid()
{
	stop_pid_fd(fd_demuxv);
}


void DVB::set_apid(ushort apid) 
{
	if (no_open) return;

        if(set_apid_fd(apid, fd_demuxa) < 0) perror("set_apid");
}

void DVB::stop_apid()
{
	stop_pid_fd(fd_demuxa);
}

void DVB::set_pcrpid(ushort pcrpid) 
{  
	if (no_open) return;

        if(set_pcrpid_fd(pcrpid, fd_demuxpcr) < 0) perror("set_pcrpid");
}

void DVB::stop_pcrpid()
{
	stop_pid_fd(fd_demuxpcr);
}


// Functions enabling the user to set the demuxes independently.

int DVB::set_vpid_fd(ushort pid, int fd)
{
	if (pid == NOPID || !pid) return 0;
        pesFilterParamsP.pid = pid;
        pesFilterParamsP.input = DMX_IN_FRONTEND;
        pesFilterParamsP.output = (dvr_enabled) ? DMX_OUT_TS_TAP :
                                                  DMX_OUT_DECODER;
        pesFilterParamsP.pes_type = (dvr_enabled==2) ? DMX_PES_OTHER :
                                                       DMX_PES_VIDEO;
        pesFilterParamsP.flags = secflags;
        return ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParamsP);
}

int DVB::set_apid_fd(ushort pid, int fd)
{
	if (pid == NOPID || !pid) return 0;
        pesFilterParamsP.pid = pid;
        pesFilterParamsP.input = DMX_IN_FRONTEND;
        pesFilterParamsP.output = (dvr_enabled) ? DMX_OUT_TS_TAP :
                                                  DMX_OUT_DECODER;
        pesFilterParamsP.pes_type = (dvr_enabled==2) ? DMX_PES_OTHER :
                                                       DMX_PES_AUDIO;
        pesFilterParamsP.flags = secflags;
        return ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParamsP);
}

int DVB::set_pcrpid_fd(ushort pid, int fd)
{
	if (pid == NOPID || !pid) return 0;
        pesFilterParamsP.pid = pid;
        pesFilterParamsP.input = DMX_IN_FRONTEND;
        pesFilterParamsP.output = (dvr_enabled) ? DMX_OUT_TS_TAP :
                                                  DMX_OUT_DECODER;
        pesFilterParamsP.pes_type = (dvr_enabled==2) ? DMX_PES_OTHER :
                                                       DMX_PES_PCR;
        pesFilterParamsP.flags = secflags;
	
	if (dvr_enabled==2) {
		ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParamsP);
		return 0;
	} else 
		return ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParamsP);
}

int DVB::set_ttpid_fd(ushort pid, int fd)
{
	if (pid == NOPID || !pid) return 0;
        pesFilterParamsP.pid = pid;
        pesFilterParamsP.input = DMX_IN_FRONTEND;
        pesFilterParamsP.output = (dvr_enabled) ? DMX_OUT_TS_TAP :
                                                  DMX_OUT_DECODER;
        pesFilterParamsP.pes_type = (dvr_enabled==2) ? DMX_PES_OTHER :
                                                       DMX_PES_TELETEXT;
        pesFilterParamsP.flags = secflags;
        return ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParamsP);
}

int DVB::set_otherpid_fd(ushort pid, int fd)
{
	if (pid == NOPID || !pid) return 0;
        pesFilterParamsP.pid = pid;
        pesFilterParamsP.input = DMX_IN_FRONTEND;
        pesFilterParamsP.output = (dvr_enabled) ? DMX_OUT_TS_TAP :
                                                  DMX_OUT_DECODER;
        pesFilterParamsP.pes_type = DMX_PES_OTHER;
        pesFilterParamsP.flags = secflags;
        return ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParamsP);
}

void DVB::stop_pid_fd(int fd)
{
	ioctl(fd, DMX_STOP, 0);
}

istream &operator>>(istream &ins, nokiaconv &x)
{
	int n=-1;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	char dummy;
	int current_sat = -1;
	int current_tp = -1;
	int dint;

	enum { NSAT=0, NNET, NTRP, NCHAN, NEND};
	static char *nokiakeys[]={
		":SAT", ":NET", ":TRP", ":CHN", ":END", NULL
	};

	while(!ins.eof()){
		streampos pos = ins.tellg();
		ins.width(MAXNAM);
		ins >> keybuf;
		n=findkey(keybuf, nokiakeys);
		if (n<0) {
			ins.seekg(pos);
			break;
		}
		switch(n){
		case NSAT:
		{
			double did;
			int id=0;
			int lnbid = 5; 
			int found = 0;

			getname(sname,ins);
			//cerr << "Satellite \"" << sname << "\"" << endl;
			for(int i=0; i < x.lnb_sat.n; i++){
				if (!strcmp(x.lnb_sat.sat_names[i],sname)){
					lnbid = x.lnb_sat.diseqc[i]; 
					id = x.lnb_sat.satid[i]; 

					found = 1;
					break;
				}
					
			}

			x.dvb->AddLNB(lnbid, 1, 
				      9750000, 10600000, 
				      11700000, 
				      lnbid, 
				      NOID, NOID);
			
			ins >> did;
			current_sat =
				x.dvb->AddSat( id, lnbid,
					       sname, 
					       10700000 , 
					       12700000);
			ins >> dummy;
			
			break;
		}

		case NNET:
			getname(sname,ins);
			//cerr << "  Network \"" << sname << "\""<< endl; 
			ins >> dint;
			break;

		case NTRP:
		{
			Transponder trans;

			trans.inversion = INVERSION_OFF;
			ins >> dec >> trans.id;
			ins >> trans.freq;
			ins >> trans.srate;
			ins >> dint;
			ins >> dummy;
			if (dummy == 'H') trans.pol = 1;
			if (dummy == 'V') trans.pol = 0;
			ins >> dint;

			trans.satid = x.dvb->sats[current_sat].id;
			trans.type = FE_QPSK;
			trans.freq *= 10;
			trans.srate *= 100;

			ins >> dint;
			ins >> dummy;
			ins >> dint;

			switch (dint){
			case 2:
				trans.fec = FEC_1_2;
				break;
			case 3:
				trans.fec = FEC_2_3;
				break;
			case 4:
				trans.fec = FEC_3_4;
				break;
			case 6:
				trans.fec = FEC_5_6;
				break;
			case 8:
				trans.fec = FEC_7_8;
				break;
			}

			current_tp = x.dvb->AddTP(trans);
			//cerr << "    Transponder "<< trans.id  << endl;
			break;
		}

		case NCHAN:
		{
			Channel chan;
			int cnum;

			getname(sname,ins);
			strncpy(chan.name, sname, maxname);
			ins >> chan.pnr;
			ins >> dummy;
			if (dummy == 'T'){
				ins.ignore(20, ':');
				ins.seekg(ins.tellg()-streampos(1));

				chan.satid = x.dvb->sats[current_sat].id;
				chan.tpid = x.dvb->tps[current_tp].id;
				cnum = x.dvb->AddChannel(chan);

				//cerr << "      Channel "<< sname  
				//     << " (" << cnum << ")" << endl;
			} else 	{
				if (dummy == 'R'){
					ins.ignore(20, ':');
					ins.seekg(ins.tellg()-streampos(1));
				} else {
					ins.seekg(pos);
					ins.ignore(80,0x0a);
				}
			}
			break;
		}

		case NEND:
			//cerr << "ALL DONE" << endl;
			return ins;
		}		
	}
	return ins;
}


#define SATCOLEN 129
istream &operator>>(istream &ins, satcoconv &x)
{
	char satline[SATCOLEN];
	char posn[5];
	char freqn[10];
	char sname[19];
	int current_sat = -1;
	int current_tp = -1;

	while(!ins.eof()){
		Transponder trans;
		int id=0;
		int lnbid=0;
		int found = 0;

		trans.inversion = INVERSION_OFF;
		ins.get(satline,SATCOLEN); // get full satco line
		if (strncmp(satline,"SATCODX103",10)){
			if (ins.eof()) return ins;
			cerr << "Wrong SATCODX format: " << endl;
			return ins;
		}

		if (satline[28]!='T') continue;
		if (strncmp(satline+29,"MPG2",4) ) continue;

		strncpy(sname,satline+10,18);
		sname[18]=0;

		for(int i=0; i < x.dvb->num[SAT]; i++){
			if (!strncmp(x.dvb->sats[i].name, sname,18)){
				lnbid = x.dvb->sats[i].lnbid; 
				id = x.dvb->sats[i].id; 
				found = 1;
				break;
			}
		}
		
		if (!found){
			lnbid = x.nlnb++;
			x.dvb->AddLNB(lnbid, 1, 
				      9750000, 10600000, 
				      11700000, 
				      lnbid, 
				      NOID, NOID);

			strncpy(posn,satline+51,4);
			posn[4]=0;
			id = strtol(posn,(char **)NULL,16);
			current_sat =
				x.dvb->AddSat( id, lnbid,
					       sname, 
					       10700000 , 
					       12700000);
		}

		trans.id = NOID;
		switch (satline[42]){
		case '1':
		case '3':
			trans.pol = 1;
			break;
		case '0':
		case '2':
			trans.pol = 0;
		}

		trans.satid = x.dvb->sats[current_sat].id;
		trans.type = FE_QPSK;

		strncpy(freqn,satline+33,9);
		freqn[8]=0;
		trans.freq = strtol(freqn,(char **)NULL,10)*10;

		strncpy(freqn,satline+69,5);
		freqn[5]=0;
		trans.srate = strtol(freqn,(char **)NULL,10)*1000;

		switch (satline[74]){
		case '0':
			trans.fec = FEC_AUTO;
			break;
		case '1':
			trans.fec = FEC_1_2;
			break;
		case '2':
			trans.fec = FEC_2_3;
			break;
		case '3':
			trans.fec = FEC_3_4;
			break;
		case '5':
			trans.fec = FEC_5_6;
			break;
		case '7':
			trans.fec = FEC_7_8;
			break;
		}

		found = 0;
		for(int i=0; i < x.dvb->num[TRANS]; i++){
			if (x.dvb->tps[i].freq == trans.freq &&
			    x.dvb->tps[i].pol == trans.pol){
				current_tp = x.dvb->tps[i].id; 
				found = 1;
				break;
			}
		}
		if (!found)
			current_tp = x.dvb->AddTP(trans);

		
		Channel chan;
		int cnum;

		strncpy(chan.name, satline+43, 8);
		strncpy(chan.name+8, satline+115, 12);
		chan.name[20] = 0;

		strncpy(freqn,satline+88,5);
		freqn[5]=0;
		chan.pnr = strtol(freqn,(char **)NULL,10)/10;

		chan.satid = x.dvb->sats[current_sat].id;
		chan.tpid = x.dvb->tps[current_tp].id;
		cnum = x.dvb->AddChannel(chan);
	}
	return ins;
}

istream &operator>>(istream &ins, vdrconv &x)
{
	char sname[MAXNAM];

	while (!ins.eof()){
		Transponder trans;
		int current_sat = -1;
		int current_tp = -1;
		Channel chan;
		string s;
		char *namebuf = NULL;
		char *sourcebuf = NULL;
		char *parambuf = NULL;
		char *vpidbuf = NULL;
		char *apidbuf = NULL;
		char *caidbuf = NULL;
		int freq, srate;
		int tpid;
		int sid, nid, tid,rid;
		int id=0;
		int lnbid=0;
		int found = 0;
		int nlen = 0;
		int cnum;
		int nlnb=0;
		int nsat=0;

		getline(ins,s);

		int fields = sscanf(s.c_str(), 
				    "%a[^:]:%d :%a[^:]:%a[^:] :%d :%a[^:]:%a[^:]:%d :%a[^:]:%d :%d :%d :%d ", 
				    &namebuf, &freq, &parambuf, &sourcebuf, &srate, &vpidbuf, &apidbuf, &tpid,&caidbuf, &sid, &nid, &tid, &rid);
		
		if (fields <9) continue;
		trans.srate = srate*1000;
		trans.fec = FEC_AUTO;
		trans.id = NOID;

		nlen = strlen(namebuf);
		if (nlen > MAXNAM) nlen = MAXNAM-1;
		for (int k=0; k< nlen; k++){
			if(namebuf[k] == ',') {
				nlen = k;
				break;
			}
		}

		if(caidbuf[0] != '0') chan.type=1;
		dvb2txt(chan.name, namebuf, nlen);
		chan.name[nlen] = 0;
		
		chan.pnr = sid;
		{
			int vpid=0;
			int pcrpid=0;
			int apid1=0;
			int apid2=0;
			int dpid1=0;
			int dpid2=0;

			char *p = strchr(vpidbuf, '+');
			if (p)
				*p++ = 0;
			sscanf(vpidbuf, "%d", &vpid);
			if (p) {
				sscanf(p, "%d", &pcrpid);
				chan.pcrpid = (uint16_t)pcrpid;
			}
			chan.vpid = (uint16_t)vpid;
			
			p = strchr(apidbuf, ';');
			if (p)
				*p++ = 0;
			sscanf(apidbuf, "%d ,%d ", &apid1, &apid2);
			chan.apids[0] = apid1;
			chan.apidnum = 1;
			if (apid2){
				chan.apidnum = 2;
				chan.apids[1] = apid2;
			} 
			if (p){
				sscanf(p, "%d ,%d ", &dpid1, &dpid2);
				chan.ac3pid = dpid1;
			}
		}

		switch(sourcebuf[0]){
		case 'S':
			x.dvb->front_type = FE_QPSK;
			trans.type = FE_QPSK;
			trans.freq = freq*1000;
			switch(parambuf[0]){
			case 'h':
			case 'H':
				trans.pol = 1;
				break;
			case 'v':
			case 'V':
				trans.pol = 0;
				break;
			}
			
			nlen = strlen(sourcebuf+1)-1;
			if (nlen > MAXNAM) nlen = MAXNAM-1;
			strncpy(sname,sourcebuf+1,nlen);
			sname[nlen]=0;
			
			for(int i=0; i < x.dvb->num[SAT]; i++){
				if (!strncmp(x.dvb->sats[i].name, sname,nlen)){
					lnbid = x.dvb->sats[i].lnbid; 
					id = x.dvb->sats[i].id; 
					found = 1;
					current_sat = i;
					break;
				}
			}
		
			if (!found){
				lnbid = nlnb++;
				x.dvb->AddLNB(lnbid, 1, 
					      9750000, 10600000, 
					      11700000, 
					      lnbid, 
					      NOID, NOID);
				
				id = nsat++;
				current_sat =
					x.dvb->AddSat( id, lnbid,
						       sname, 
						       10700000 , 
						       12700000);
			}
			
			trans.satid = x.dvb->sats[current_sat].id;
			found = 0;
			for(int i=0; i < x.dvb->num[TRANS]; i++){
				if (x.dvb->tps[i].freq == trans.freq &&
				    x.dvb->tps[i].pol == trans.pol){
					current_tp = i;
					found = 1;
					break;
				}
			}
			if (!found)
				current_tp = x.dvb->AddTP(trans);
  			break;

		case 'C':
			x.dvb->front_type = FE_QAM;
			trans.type = FE_QAM;
			trans.freq = freq*1000000;



			{
				char *s = parambuf;
				char *b;

			while (s && *s) {
				switch(*s){
				case 'M':
				case 'm':
					switch(strtol(s+1,&b,10)){
						
					case 0:
						trans.qam = QPSK;
						break;
					case 16:
						trans.qam = QAM_16;
						break;
					case 32:
						trans.qam = QAM_32;
						break;
					case 64:
						trans.qam = QAM_64;
						break;
					case 128:
						trans.qam = QAM_128;
						break;
					case 256:
						trans.qam = QAM_256;
						break;
					default:
						trans.qam = QAM_AUTO;
						break;

					}
					s = b;
					break;


				default: 
					cerr << "Error reading parameters " << s << endl;
					break;
				}
			}
			}


			if (x.dvb->num[SAT]){
				found =1; 
				current_sat = 0;
			}
		
			if (!found){
				lnbid = nlnb++;
				x.dvb->AddLNB(lnbid, 2, 
					      0, 0, 0, lnbid, NOID, NOID);
				
				id = nsat++;
				current_sat =
					x.dvb->AddSat( id, lnbid,
						       "DVBC", 
						       350000000 , 
						       500000000);
			}
			
			trans.satid = x.dvb->sats[current_sat].id;
			found = 0;
			for(int i=0; i < x.dvb->num[TRANS]; i++){
				if (x.dvb->tps[i].freq == trans.freq){
					current_tp = i;
					found = 1;
					break;
				}
			}
			if (!found)
				current_tp = x.dvb->AddTP(trans);

			
			break;

		case 'T':
			x.dvb->front_type = FE_OFDM;
			trans.type = FE_OFDM;
 			trans.freq = freq;
			{
				char *s = parambuf;
				char *b;

			while (s && *s) {
				switch(*s){
				case 'Y':
				case 'y':
					switch(strtol(s+1,&b,10)){
					case 0:
						trans.hierarchy = HIERARCHY_NONE;
						break;
					case 1:
						trans.hierarchy = HIERARCHY_1;
						break;
					case 2:
						trans.hierarchy = HIERARCHY_2;
						break;
					case 4:
						trans.hierarchy = HIERARCHY_4;
						break;
					default:
						trans.hierarchy = HIERARCHY_AUTO;
						break;
					}
					s = b;
					break;

				case 'M':
				case 'm':
					switch(strtol(s+1,&b,10)){
						
					case 0:
						trans.mod = QPSK;
						break;
					case 16:
						trans.mod = QAM_16;
						break;
					case 32:
						trans.mod = QAM_32;
						break;
					case 64:
						trans.mod = QAM_64;
						break;
					case 128:
						trans.mod = QAM_128;
						break;
					case 256:
						trans.mod = QAM_256;
						break;
					default:
						trans.mod = QAM_AUTO;
						break;

					}
					s = b;
					break;

				case 'I':
				case 'i':
					if (atoi(s+1)){
						trans.inversion = INVERSION_ON;
					} else {
						trans.inversion = INVERSION_ON;
					}
					s+=2;
					break;

				case 'b':
				case 'B':
					switch(strtol(s+1,&b,10)){

					case 6:
						trans.band = BANDWIDTH_6_MHZ;
						break;
					case 7:
						trans.band = BANDWIDTH_7_MHZ;
						break;
					case 8:
						trans.band = BANDWIDTH_8_MHZ;
						break;
						
					default:
						trans.band = BANDWIDTH_AUTO;
						break;
					}
					s = b;
					break;
					
				case 'c':
				case 'C':
					switch(strtol(s+1,&b,10)){
					case 0:
						trans.hp_rate = FEC_NONE;
						break;
						
					case 12:
						trans.hp_rate = FEC_1_2;
						break;
						
					case 23:
						trans.hp_rate = FEC_2_3;
						break;
						
					case 34:
						trans.hp_rate = FEC_3_4;
						break;
						
					case 45:
						trans.hp_rate = FEC_4_5;
						break;
						
					case 56:
						trans.hp_rate = FEC_5_6;
						break;
						
					case 67:
						trans.hp_rate = FEC_6_7;
						break;
						
					case 78:
						trans.hp_rate = FEC_7_8;
						break;
						
					case 89:
						trans.hp_rate = FEC_8_9;
						break;
						
					default:
						trans.hp_rate = FEC_AUTO;
						break;
					}
					s = b;
					break;
					
				case 'D':
				case 'd':
					switch(strtol(s+1,&b,10)){
					case 0:
						trans.lp_rate = FEC_NONE;
						break;
						
					case 12:
						trans.lp_rate = FEC_1_2;
						break;
						
					case 23:
						trans.lp_rate = FEC_2_3;
						break;
						
					case 34:
						trans.lp_rate = FEC_3_4;
						break;
						
					case 45:
						trans.lp_rate = FEC_4_5;
						break;
						
					case 56:
						trans.lp_rate = FEC_5_6;
						break;
						
					case 67:
						trans.lp_rate = FEC_6_7;
						break;
						
					case 78:
						trans.lp_rate = FEC_7_8;
						break;
						
					case 89:
						trans.lp_rate = FEC_8_9;
						break;
						
					default:
						trans.lp_rate = FEC_AUTO;
						break;
					}
					s = b;
					break;
					

				case 'G':
				case 'g':
					switch(strtol(s+1,&b,10)){
					case 4:
						trans.guard = GUARD_INTERVAL_1_4;
						break;
					case 8:
						trans.guard = GUARD_INTERVAL_1_8;
						break;
					case 16:
						trans.guard = GUARD_INTERVAL_1_16;
						break;
					case 32:
						trans.guard = GUARD_INTERVAL_1_32;
						break;
					default:
						trans.guard = GUARD_INTERVAL_AUTO;
						break;
					}
					s = b;
					break;

				case 'T':
					switch(strtol(s+1,&b,10)){
					case 2:
						trans.transmode = TRANSMISSION_MODE_2K;
						break;
					case 8:
						trans.transmode = TRANSMISSION_MODE_8K;
						break;
					default:
						trans.transmode = TRANSMISSION_MODE_AUTO;
						break;
					}
					s=b;
					break;

				default: 
					cerr << "Error reading parameters " << s << endl;
					break;
				}
			}
			}

			if (x.dvb->num[SAT]){
				found =1; 
				current_sat = 0;
			}
		
			if (!found){
				lnbid = nlnb++;
				x.dvb->AddLNB(lnbid, 2, 
					      0, 0, 0, lnbid, NOID, NOID);
				
				id = nsat++;
				current_sat =
					x.dvb->AddSat( id, lnbid,
						       "DVBT", 
						       630000000 , 
						       900000000);
			}
			
			trans.satid = x.dvb->sats[current_sat].id;
			found = 0;
			for(int i=0; i < x.dvb->num[TRANS]; i++){
				if (x.dvb->tps[i].freq == trans.freq){
					current_tp = i;
					found = 1;
					break;
				}
			}
			if (!found)
				current_tp = x.dvb->AddTP(trans);

			
			break;
			
		}
		
		chan.satid = x.dvb->sats[current_sat].id;
		chan.tpid = x.dvb->tps[current_tp].id;
		cnum = x.dvb->AddChannel(chan);
		
		free(namebuf);
		free(sourcebuf);
		free(parambuf);
		free(vpidbuf);
		free(apidbuf);
		free(caidbuf);
	}

	return ins;

}

istream &operator>>(istream &ins, zapconv &x)
{
	while (!ins.eof()){
		Transponder trans;
		int current_sat = -1;
		int current_tp = -1;
		Channel chan;
		char *namebuf = NULL;
		int id=0;
		int lnbid=0;
		int found = 0;
		int nlen = 0;
		int nsat=0;
		string s;

		getline(ins,s);
		

		switch(x.dvb->front_type){

		case FE_QPSK:
		{
			char sname[MAXNAM];
			int freq;
			char *pol;
			int srate, diseqc, vpid, apid,st, sid;
			
			int fields = sscanf(s.c_str(), 
					    "%a[^:]:%d :%a[^:]:%d :%d :%d :%d :%d :%d", 
					    &namebuf, &freq, &pol, &diseqc, &srate,
					    &vpid,&apid,&st,&sid);
			
			if (fields <7) continue;
			trans.type = FE_QPSK;
			trans.freq = freq*1000;
			switch(pol[0]){
			case 'h':
			case 'H':
				trans.pol = 1;
				break;
			case 'v':
			case 'V':
				trans.pol = 0;
				break;
			}
			trans.srate = srate*1000;
			chan.apids[0] = apid;
			chan.apidnum= 1;
			chan.vpid = vpid;
			chan.pnr = sid;
			
			nlen = strlen(namebuf);
			if (nlen > MAXNAM) nlen = MAXNAM-1;
			
			dvb2txt(chan.name, namebuf, nlen);
			chan.name[nlen] = 0;
			
			free(pol);
			
			sprintf(sname,"SAT%d",nsat);
			for(int i=0; i < x.dvb->num[SAT]; i++){
				if (!strncmp(x.dvb->sats[i].name, sname,nlen)){
					lnbid = x.dvb->sats[i].lnbid; 
					id = x.dvb->sats[i].id; 
					found = 1;
					current_sat = i;
					break;
				}
			}
			
			if (!found){
				lnbid = diseqc;
				x.dvb->AddLNB(lnbid, 1, 
					      9750000, 10600000, 
					      11700000, 
					      lnbid, 
					      NOID, NOID);
				
				id = nsat++;
				current_sat =
					x.dvb->AddSat( id, lnbid,
						       sname, 
						       10700000 , 
						       12700000);
			}
			
			trans.satid = x.dvb->sats[current_sat].id;
			found = 0;
			for(int i=0; i < x.dvb->num[TRANS]; i++){
				if (x.dvb->tps[i].freq == trans.freq &&
				    x.dvb->tps[i].pol == trans.pol){
					current_tp = i;
					found = 1;
					break;
				}
			}
			
			if (!found)
				current_tp = x.dvb->AddTP(trans);
			
		}
		break;

		case FE_QAM:
		{
			
			int freq;
			char *inv, *fec, *qam;
			int srate, vpid, apid;
			
			int fields = sscanf(s.c_str(), 
					    "%a[^:]:%d :%a[^:]:%d :%a[^:] :%a[^:] :%d :%d", 
					    &namebuf, &freq, &inv, &srate, &fec, &qam,
					    &vpid,&apid);
			
			if (fields <7) continue;
			trans.type = FE_QAM;
			trans.freq = freq;

			trans.srate = srate;
			chan.apids[0] = apid;
			chan.apidnum= 1;
			chan.vpid = vpid;
			chan.pnr = NOID;
			  
			trans.inversion = INVERSION_AUTO;
			for (int in = 0; in < 3; in++){
				if (! strncmp(inv_name[in], inv, strlen(inv_name[in]))){
					switch(in){
					case 0:
						trans.inversion = INVERSION_OFF;
						break;
					case 1:
						trans.inversion = INVERSION_ON;
						break;
					case 2:
						trans.inversion = INVERSION_AUTO;
						break;
					}
					break;
				}
			}

			trans.fec = FEC_AUTO;
			for (int fe = 0; fe < 10; fe++){
				if (! strncmp(fec_name[fe], fec, strlen(fec_name[fe]))){
					switch(fe){
					case 0:
						trans.fec = FEC_NONE;
						break;
					case 1:
						trans.fec = FEC_1_2;
						break;

					case 2:
						trans.fec = FEC_2_3;
						break;

					case 3:
						trans.fec = FEC_3_4;
						break;
						
					case 4:
						trans.fec = FEC_4_5;
						break;

					case 5:
						trans.fec = FEC_5_6;
						break;

					case 6:
						trans.fec = FEC_6_7;
						break;

					case 7:
						trans.fec = FEC_7_8;
						break;
						
					case 8:
						trans.fec = FEC_8_9;
						break;

					case 9:
						trans.fec = FEC_AUTO;
						break;
					}
					break;
				}
			}

			for (int qa = 0; qa < 10; qa++){
				if (! strncmp(qam_name[qa], qam, strlen(qam_name[qa]))){
					switch(qa){
					case 0:
						trans.qam = QPSK;
						break;
					case 1:
						trans.qam = QAM_16;
						break;

					case 2:
						trans.qam = QAM_32;
						break;

					case 3:
						trans.qam = QAM_64;
						break;
						
					case 4:
						trans.qam = QAM_128;
						break;

					case 5:
						trans.qam = QAM_256;
						break;

					case 6:
						trans.qam = QAM_AUTO;
						break;

					}
					break;
				}
			}

			nlen = strlen(namebuf);
			if (nlen > MAXNAM) nlen = MAXNAM-1;
			  
			dvb2txt(chan.name, namebuf, nlen);
			chan.name[nlen] = 0;
			  
			free(inv);
			free(fec);
			free(qam);


			if (x.dvb->num[SAT]){
				found =1; 
				current_sat = 0;
			}
		
			if (!found){
				lnbid = 1;
				x.dvb->AddLNB(lnbid, 2, 
					      0, 0, 0, lnbid, NOID, NOID);
				
				id = nsat++;
				current_sat =
					x.dvb->AddSat( id, lnbid,
						       "DVBC", 
						       350000000 , 
						       500000000);
			}
			
			trans.satid = x.dvb->sats[current_sat].id;
			found = 0;
			for(int i=0; i < x.dvb->num[TRANS]; i++){
				if (x.dvb->tps[i].freq == trans.freq){
					current_tp = i;
					found = 1;
					break;
				}
			}
			if (!found)
				current_tp = x.dvb->AddTP(trans);

		}
		break;
		
		case FE_OFDM:
		{
			
			int freq;
			char *inv, *fech, *fecl, *qam, *transm, *guard, *hier, *band;
			int vpid, apid, sid;
			
			int fields = sscanf(s.c_str(), 
					    "%a[^:]:%d :%a[^:]:%a[^:] :%a[^:] :%a[^:] :%a[^:] :%a[^:] :%a[^:] :%a[^:] :%d :%d :%d", 
					    &namebuf, &freq, &inv, &band, &fech,&fecl, &qam, 
					    &transm, &guard, &hier, 
					    &vpid,&apid,&sid);
			
			if (fields <12) continue;
			trans.type = FE_QAM;
			trans.freq = freq;

			chan.apids[0] = apid;
			chan.apidnum= 1;
			chan.vpid = vpid;
			chan.pnr = sid;
			
			  
			trans.inversion = INVERSION_AUTO;
			for (int in = 0; in < 3; in++){
				if (! strncmp(inv_name[in], inv, strlen(inv_name[in]))){
					switch(in){
					case 0:
						trans.inversion = INVERSION_OFF;
						break;
					case 1:
						trans.inversion = INVERSION_ON;
						break;
					case 2:
						trans.inversion = INVERSION_AUTO;
						break;
					}
					break;
				}
			}


			for (int ba = 0; ba < 4; ba++){
				if (! strncmp(bw_name[ba], band, strlen(bw_name[ba]))){
					switch(ba){
        
					case 0:
						trans.band = BANDWIDTH_8_MHZ;
						break;
					case 1:
						trans.band = BANDWIDTH_7_MHZ;
						break;
					case 2:
						trans.band = BANDWIDTH_6_MHZ; 
						break;
						
					case 3:
						trans.band = BANDWIDTH_AUTO;
						break;
					}
					break;
				}
			}

			trans.hp_rate = FEC_AUTO;
			for (int fe = 0; fe < 10; fe++){
				if (! strncmp(fec_name[fe], fech, strlen(fec_name[fe]))){
					switch(fe){
					case 0:
						trans.hp_rate = FEC_NONE;
						break;
					case 1:
						trans.hp_rate = FEC_1_2;
						break;

					case 2:
						trans.hp_rate = FEC_2_3;
						break;

					case 3:
						trans.hp_rate = FEC_3_4;
						break;
						
					case 4:
						trans.hp_rate = FEC_4_5;
						break;

					case 5:
						trans.hp_rate = FEC_5_6;
						break;

					case 6:
						trans.hp_rate = FEC_6_7;
						break;

					case 7:
						trans.hp_rate = FEC_7_8;
						break;
						
					case 8:
						trans.hp_rate = FEC_8_9;
						break;

					case 9:
						trans.hp_rate = FEC_AUTO;
						break;
					}
					break;
				}
			}

			trans.lp_rate = FEC_AUTO;
			for (int fe = 0; fe < 10; fe++){
				if (! strncmp(fec_name[fe], fecl, strlen(fec_name[fe]))){
					switch(fe){
					case 0:
						trans.lp_rate = FEC_NONE;
						break;
					case 1:
						trans.lp_rate = FEC_1_2;
						break;

					case 2:
						trans.lp_rate = FEC_2_3;
						break;

					case 3:
						trans.lp_rate = FEC_3_4;
						break;
						
					case 4:
						trans.lp_rate = FEC_4_5;
						break;

					case 5:
						trans.lp_rate = FEC_5_6;
						break;

					case 6:
						trans.lp_rate = FEC_6_7;
						break;

					case 7:
						trans.lp_rate = FEC_7_8;
						break;
						
					case 8:
						trans.lp_rate = FEC_8_9;
						break;

					case 9:
						trans.lp_rate = FEC_AUTO;
						break;
					}
					break;
				}
			}

			for (int qa = 0; qa < 10; qa++){
				if (! strncmp(qam_name[qa], qam, strlen(qam_name[qa]))){
					switch(qa){
					case 0:
						trans.mod = QPSK;
						break;
					case 1:
						trans.mod = QAM_16;
						break;

					case 2:
						trans.mod = QAM_32;
						break;

					case 3:
						trans.mod = QAM_64;
						break;
						
					case 4:
						trans.mod = QAM_128;
						break;

					case 5:
						trans.mod = QAM_256;
						break;

					case 6:
						trans.mod = QAM_AUTO;
						break;

					}
					break;
				}
			}

			for (int ga = 0; ga < 5; ga++){
				if (! strncmp(guard_name[ga], guard, strlen(guard_name[ga]))){
					switch(ga){
        
					case 0:
						trans.guard = GUARD_INTERVAL_1_32;
						break;
					case 1:
						trans.guard = GUARD_INTERVAL_1_16;
						break;

					case 2:
						trans.guard = GUARD_INTERVAL_1_8; 
						break;

					case 3:
						trans.guard = GUARD_INTERVAL_1_4;
						break;
						
					case 4:
						trans.guard = GUARD_INTERVAL_AUTO;
						break;
					}
					break;
				}
			}

			for (int hi = 0; hi < 5; hi++){
				if (! strncmp(hierarchy_name[hi], hier, 
					      strlen(hierarchy_name[hi]))){
					switch(hi){

					case 0:
						trans.hierarchy = HIERARCHY_NONE;
						break;
					case 1:
						trans.hierarchy = HIERARCHY_1;
						break;

					case 2:
						trans.hierarchy = HIERARCHY_2;
						break;

					case 3:
						trans.hierarchy = HIERARCHY_4;
						break;
						
					case 4:
						trans.hierarchy = HIERARCHY_AUTO;
						break;
						
					}
					break;
				}
			}

			
			for (int tr = 0; tr < 3; tr++){
				if (! strncmp(mode_name[tr], transm, strlen(mode_name[tr]))){
					switch(tr){
					case 0:
						trans.transmode = TRANSMISSION_MODE_2K;
						break;
					case 1:
						trans.transmode = TRANSMISSION_MODE_8K;
						break;
						
					case 2:
						trans.transmode = TRANSMISSION_MODE_AUTO;
						break;
					}
					break;
				}
			}
			
			nlen = strlen(namebuf);
			if (nlen > MAXNAM) nlen = MAXNAM-1;
			  
			dvb2txt(chan.name, namebuf, nlen);
			chan.name[nlen] = 0;
			  

			free(inv);
			free(band);
			free(fech);
			free(fecl);
			free(qam);
			free(transm);
			free(guard);
			free(hier);


			if (x.dvb->num[SAT]){
				found =1; 
				current_sat = 0;
			}
		
			if (!found){
				lnbid = 1;
				x.dvb->AddLNB(lnbid, 2, 
					      0, 0, 0, lnbid, NOID, NOID);
				
				id = nsat++;
				current_sat =
					x.dvb->AddSat( id, lnbid,
						       "DVBT", 
						       630000000 , 
						       900000000);
			}
			
			trans.satid = x.dvb->sats[current_sat].id;
			found = 0;
			for(int i=0; i < x.dvb->num[TRANS]; i++){
				if (x.dvb->tps[i].freq == trans.freq){
					current_tp = i;
					found = 1;
					break;
				}
			}
			if (!found)
				current_tp = x.dvb->AddTP(trans);
			
		}		
			
		break;

			
		}
		
		chan.satid = x.dvb->sats[current_sat].id;
		chan.tpid = x.dvb->tps[current_tp].id;
		x.dvb->AddChannel(chan);
	}
	
	return ins;
		
}

static int get_keylen(istream &ins, char *keybuf)
{
	streampos pos = ins.tellg();
	int klen = strlen(keybuf);
	if (klen>2 && keybuf[1]!= '/' &&
	    keybuf[0]=='<' && keybuf[klen-1]=='>'){
		keybuf[klen-2]='\0';
		klen--;
		ins.seekg(pos-streampos(2));
	}
	return klen;
}

static int find_xml_key(istream &ins, char *keybuf, char *keys[])
{
	char *s;
	int n;
	streampos pos = ins.tellg();
	ins.width(MAXNAM);
	ins >> keybuf;
	int klen = get_keylen(ins, keybuf);
	s=keybuf;
	while (s[0] != '=' && s != keybuf+klen)s++;
	s[0]=0;
	ins.seekg(pos + streampos((s-keybuf) +1) ); // go past =
	n=findkey(keybuf, keys);
	if (n<0) {
		ins.seekg(pos);
		cerr << "Unknown tag: " << keybuf << endl;
	}
	return n;
}

int xmlconv::read_sat(istream &ins, int csat)
{
	int n=-1;
	int lnbid=-1;
	int satid;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	enum { XSATN, XSLNB, XSATID, XTRANS, XSATEND, XEND, XNEND};
	static char *xsat[]={
		"name", "lnb", "id", "<transponder", "</satellite>",
		">", "/>", NULL
	};
	while(!ins.eof()){
		if ( (n = find_xml_key( ins, keybuf, xsat)) < 0) break;
		switch(n){
		case XSATN:
			getname(sname,ins);			
			break;

		case XSLNB:
			ins >> satid;
			break;

		case XTRANS:
			if (csat >= 0)
				read_trans(ins, csat);
			else
				return -1;
			break;

		case XSATID:
			ins >> satid;
			break;

		case XSATEND:
			return 0;
			break;

		case XEND:
			if (satid >=0 && lnbid >= 0)
				csat =	dvb->AddSat(satid, 
						    lnbid,
						    sname, 
						    10700000 , 
						    12700000);
			break;

		case XNEND:
			return 0;
			break;

		default:
			skip_tag(ins,keybuf);
			break;
		}
	}

	return 0;
}

int xmlconv::read_trans(istream &ins, int csat)
{
	int n=-1;
	int ctp=-1;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	enum { XTYPE=0, XFREQ, XSRATE, XPOL, XFEC, XSERV, 
	       XTRANSEND, XEND, XNEND };
	static char *xtrans[]={
		"type", "freq", "srate", "polarity", "fec", 
		"<service", "</transponder>", 
		">", "/>", NULL
	};
	Transponder trans;
	trans.satid = dvb->sats[csat].id;
	trans.fec = FEC_AUTO;
	trans.id = NOID;
	trans.inversion = INVERSION_OFF;


	while(!ins.eof()){
		if ( (n = find_xml_key( ins, keybuf, xtrans)) < 0) break;
		switch(n){
		case XTYPE:
			getname(sname,ins);
			switch(sname[0]){
			case 'S':
				trans.type = FE_QPSK;
				break;
			case 'T':
				trans.type = FE_OFDM;
				break;
			case 'C':
				trans.type = FE_QAM;
				break;
			}
			break;

		case XFREQ:
			getname(sname,ins);
			trans.freq=atoi(sname);
			break;

		case XSRATE:
			getname(sname,ins);
			trans.srate=atoi(sname);
			break;

		case XPOL:
			getname(sname,ins);
			if (sname[0] == 'H') trans.pol = 1;
			if (sname[0] == 'V') trans.pol = 0;
			break;

		case XFEC:
			int dint;
			getname(sname,ins);
			dint = atoi(sname);

			switch (dint){
			case 2:
				trans.fec = FEC_1_2;
				break;
			case 3:
				trans.fec = FEC_2_3;
				break;
			case 4:
				trans.fec = FEC_3_4;
				break;
			case 6:
				trans.fec = FEC_5_6;
				break;
			case 8:
				trans.fec = FEC_7_8;
				break;
			}

			break;


		case XSERV:
			if (ctp>=0)
				read_serv(ins,ctp,csat);
			break;

		case XTRANSEND:
			return 0;
			break;

		case XEND:
			ctp = dvb->AddTP(trans);
			break;

		case XNEND:
			return 0;
			break;
		default:
			skip_tag(ins,keybuf);
			break;

		}
	}

	return 0;
}

int xmlconv::read_serv(istream &ins, int ctp, int csat)
{
	int n=-1;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	enum { XSID=0, XCA, XDESC, XSTREAM, 
	       XSERVEND, XEND, XNEND };
	static char *xserv[]={
		"id", "ca", 
		"<description", 
		"<stream", "</service>",  		
		">", "/>", 
		"<ca_descriptor", "<descriptor",
		"<country_availability", "<ca_system_id", "<time_shifted_copy_of",
		NULL
	};
	Channel chan;
	int nchan=-1;

	chan.satid = dvb->sats[csat].id;
	chan.tpid = dvb->tps[ctp].id;

	while(!ins.eof()){
		if ( (n = find_xml_key( ins, keybuf, xserv)) < 0) break;
		switch(n){
		case XSID:
			getname(sname,ins);
			chan.pnr = atoi(sname);
			nchan = dvb->AddChannel(chan);
			break;

		case XCA:
			getname(sname,ins);
			if (nchan >= 0)
				dvb->chans[nchan].type = atoi(sname);
			else
				chan.type = atoi(sname);
			break;

		case XDESC:
			if (nchan>=0)
				read_desc(ins, nchan);
			else
				return -1;
			break;

		case XSTREAM: 
			if (nchan>=0)
				read_stream(ins,nchan);
			else
				return -1;
			break;

		case XSERVEND: 
			return 0;
			break;

		case XEND: 
			break;

		case XNEND:
			return 0;
			break;
		default:
			skip_tag(ins,keybuf);
			break;
		}
	}

	return 0;
}

int xmlconv::read_desc(istream &ins, int nchan)
{
	int n=-1;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	enum { XTAG=0, XTYPE, XPROV, XSNAME, XDESCEND, 
	       XEND, XNEND};
	static char *xdesc[]={
		"tag", "type", "provider_name", "service_name", 
		"</description>", ">", "/>", NULL
	};

	while(!ins.eof()){
		if ( (n = find_xml_key( ins, keybuf, xdesc)) < 0) break;
		switch(n){
		case XTAG:
			getname(sname,ins);
			break;

		case XTYPE:
			getname(sname,ins);
			break;

		case XPROV:
			getname(sname,ins);
			break;

		case XSNAME:
			getname(sname,ins);
			dvb2txt(dvb->chans[nchan].name,sname,MAXNAM);
			break;

		case XDESCEND:
			return 0;
			break;

		case XEND:
			break;

		case XNEND:
			return 0;
			break;

		default:
			skip_tag(ins,keybuf);
			break;
		}
	}

	return 0;
}

int xmlconv::read_stream(istream &ins, int nchan)
{
	int n=-1;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	int type = -1;
	int apids = 0;
	uint16_t pid = NOPID;
	enum { XTYPE, XPID, XISO, XSTREAMEND, XEND, XNEND };
	static char *xstream[]={
		"type", "pid", "<iso_639",
		"</stream>", ">", "/>", 
		"<ca_descriptor","<descriptor","<teletext","<stream_id", 
		"<canal_radio", "<audio_info", "<description", "<ac3_descriptor",
		NULL
	};

	while(!ins.eof()){
		if ( (n = find_xml_key( ins, keybuf, xstream)) < 0) break;
		switch(n){
		case XTYPE:
			getname(sname,ins);
			type = atoi(sname);
			break;

		case XPID:
			getname(sname,ins);
			pid = atoi(sname);
			switch(type){
			case 1:
			case 2:
				if (pid != NOPID)
					dvb->chans[nchan].vpid = pid;
				break;
			case 3:
			case 4:
				if (pid == NOPID) break;
				apids = dvb->chans[nchan].apidnum;
				if (apids >= MAXAPIDS) break;
				dvb->chans[nchan].apidnum++;
				dvb->chans[nchan].apids[apids]=pid;
				break;
			case 6:
				if (pid != NOPID)
					dvb->chans[nchan].ttpid = pid;
				break;				
			}
			break;

		case XSTREAMEND:
			return 0;
			break;

		case XEND:
			break;
			
		case XNEND:
			return 0;
			break;

		case XISO:
			read_iso639(ins, nchan, apids);
			break;

		default:
			skip_tag(ins,keybuf);
			break;
		}
	}

	return 0;
}

int xmlconv::read_iso639(istream &ins, int nchan, int apids)
{
	int n=-1;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	enum { XTYPE, XLAN,XISOEND, XEND, XNEND };
	static char *xiso[]={
		"type", "language", "</iso_639>",
		">", "/>", NULL
	};

	while(!ins.eof()){
		if ( (n = find_xml_key( ins, keybuf, xiso)) < 0) break;
		switch(n){
		case XTYPE:
			getname(sname,ins);
			break;

		case XLAN:
			getname(sname,ins);
			strncpy(dvb->chans[nchan].apids_name+apids*4, sname, 4);
			break;

		case XISOEND:
			return 0;
			break;

		case XEND:
			break;
			
		case XNEND:
			return 0;
			break;

		default:
			skip_tag(ins,keybuf);
			break;
		}
	}

	return 0;
}

int xmlconv::skip_tag(istream &ins, char *tag)
{
	char sname[MAXNAM*2];
	char endtag[MAXNAM];
	int found = 0;
	streampos pos = ins.tellg();
	
	ostringstream etag;

	etag << "</" << tag+1 << ">" << ends;
	strncpy(endtag, etag.str().data(),MAXNAM);
	int tlen = strlen(endtag)-1;
	
//	cerr << "find: " << endtag << endl;
	ins.width(2*MAXNAM);
	ins >> sname;
	if (sname[0] == '>')
		while(!found){
			if (!strncmp(sname,endtag,tlen)) 
				found=1;
			else 
				ins >> sname;
		}
	else {
		ins.seekg(pos);
		ins.ignore(1000,'>');
		pos = ins.tellg();
		ins.seekg(pos-streampos(2));
		ins >> sname;
		if (sname[0] == '/')
			ins.seekg(pos);
		else 
			while(!found){
				if (!strncmp(sname,endtag,tlen)) 
					found=1;
				else
					ins >> sname;
			}

	}
	return 0;
}

istream &operator>>(istream &ins, xmlconv &x)
{
	int n=-1;
	char keybuf[MAXNAM];
	char sname[MAXNAM];
	int current_sat = -1;
	int nsat = 0;

	enum { XMLSTART=0, XSAT, XNSAT, XLNB, XEND, XNEND};
	static char *xmltags[]={
		"<?xml","<satellite", "<satellite>",
		"<lnb",
		">", "/>", NULL};

	while(!ins.eof()){
		streampos pos = ins.tellg();
		ins.width(MAXNAM);
		ins >> keybuf;
		n=findkey(keybuf, xmltags);
		if (n<0) {
			ins.seekg(pos);
			cerr << "Unknown tag: " << keybuf << endl;
			break;
		}
		switch(n){
		case XMLSTART:
			cerr << "xml start found" << endl;
			ins.ignore(80,'>');
			break;

		case XNSAT:
		{
			int clnb;
			int lnbid = 5; 
			int satid = -1;

			if (nsat > XML_MAX_SAT) break;
			strncpy(sname,x.lnb_sat.sat_names[nsat],MAXNAM);
			lnbid = x.lnb_sat.diseqc[nsat]; 
			clnb = x.dvb->AddLNB(lnbid, 1, 
				      9750000, 10600000, 
				      11700000, 
				      lnbid, 
				      NOID, NOID);
			
			satid = x.lnb_sat.satid[nsat];
			current_sat =
				x.dvb->AddSat(satid, 
					       lnbid,
					       sname, 
					       10700000 , 
					       12700000);
			nsat++;
			x.read_sat(ins, current_sat);
			break;
		}

		case XSAT:
		{
			cerr << "no sat name" << endl;
			x.read_sat(ins, -1);
			break;
		}

		case XLNB:

			break;

		default:
			x.skip_tag(ins,keybuf);
			break;
		}		
	}
	return ins;
}

int get_dvbrc(char *path, DVB &dv, int dev, int len)
{
        ifstream dvbin;

	dvbin.open(path);
	if (!dvbin){
		const char *home = getenv("HOME");
		const char *file = ".dvbrc";

		ostringstream str;

		str << home << "/" << file ;
		if (dev)
			str << "." << dev ;
		str << ends;
		strncpy(path,str.str().data(),len);
		cerr << "Using default "<< path << endl;
		dvbin.clear();
		dvbin.open(path);
	}
        if (dvbin) {
		cerr << endl;
                dvbin >> dv;
                return 1;
        } else {
		cerr << " failed" << endl;
		ostringstream str;
		
		str << "/etc/dvb/dvbrc";
		if (dev)
			str << "." << dev ;
		
		str << ends;
		strncpy(path,str.str().data(),len);
		cerr << "Using default "<< path << endl;

		dvbin.clear();
		dvbin.open(path);
		if (dvbin) {
			cerr << endl;
			dvbin >> dv;
			return 1;
		} else cerr << " failed" << endl;
	}

        return 0;
}

int set_dvbrc(char *path, DVB &dv, int dev, int len)
{
        ofstream dvbout;

	dvbout.open(path);
	if (!dvbout){
		cerr << "Using default dvbrc." << endl;
		const char *home = getenv("HOME");
		const char *file = ".dvbrc";

		ostringstream str;

		str << home << "/" << file ;
		if (dev)
			str << "." << dev ;
		str << ends;
		
		strncpy(path, str.str().data(),len);
		dvbout.clear();
		dvbout.open(path);
	}
        if (dvbout) {
                dvbout << dv;
                return 1;
        }

        return 0;
}



#define SFREQ 11700000
#define LHI 10600000
#define LLO 9750000

#define IPACKS 2048


void set_diseqc(int fdf, int snum, fe_sec_voltage_t v, fe_sec_tone_mode_t t)
{
        struct dvb_diseqc_master_cmd dcmd;
        fe_sec_mini_cmd_t b;
        int hi_lo;
        int pol;

        if (snum >= 0) fprintf(stderr,"Setting diseqc %d \n",snum);
        
        hi_lo = (t == SEC_TONE_ON) ? 1 : 0; 
        pol = (v==SEC_VOLTAGE_18) ? 2:0;
        dcmd.msg[0]=0xe0;
        dcmd.msg[1]=0x10;
        dcmd.msg[2]=0x38;
        dcmd.msg[3] = 0xf0 | (((snum* 4) & 0x0f) | hi_lo | pol);
        dcmd.msg[4]=0x00;
        dcmd.msg[5]=0x00;
        dcmd.msg_len=4;


        b = (snum &1) ? SEC_MINI_B : SEC_MINI_A;

        if (snum >= 0) ioctl(fdf, FE_SET_TONE, SEC_TONE_OFF);
        ioctl(fdf, FE_SET_VOLTAGE, v);
        if (snum >= 0) {
                usleep(15 * 1000);
                ioctl(fdf, FE_DISEQC_SEND_MASTER_CMD, &dcmd);
                usleep(15 * 1000);
                ioctl(fdf, FE_DISEQC_SEND_BURST, b);
                usleep(15 * 1000);
        }
        ioctl(fdf, FE_SET_TONE, t);
}

int tune(int fdf, uint32_t freq, uint32_t sr, fe_code_rate_t fec, 
	 fe_spectral_inversion_t inv=INVERSION_OFF)
{
	struct dvb_frontend_parameters tune;
		
	tune.frequency = freq;
	tune.inversion = inv;
	tune.u.qpsk.symbol_rate = sr;
	if (!fec)
		tune.u.qpsk.fec_inner = FEC_AUTO;
	else 
		tune.u.qpsk.fec_inner = fec;

	if (ioctl(fdf, FE_SET_FRONTEND, &tune) == -1) {
		perror("FE_SET_FRONTEND failed");
		return -1;
	}

	return 0;
}


int set_sfront(int fdf, uint32_t freq, uint32_t pol, uint32_t sr , int snum, 
	       fe_code_rate_t fec)
{
        fe_sec_voltage_t v;
        fe_sec_tone_mode_t t;
        uint32_t f;

        fprintf(stderr,"%d %d %d %d %d\n",freq,pol,sr,snum,fec);

        if (freq >= SFREQ){
                f = freq - LHI;
                t = SEC_TONE_ON;        
        } else {
                f = freq - LLO;
                t = SEC_TONE_OFF;
        }
        v = pol ? SEC_VOLTAGE_18 : SEC_VOLTAGE_13; 

        set_diseqc(fdf, snum, v, t);

        return tune(fdf, f, sr, fec);
}


void set_pes_filt(int fd,uint16_t pes_pid)
{
	struct dmx_pes_filter_params pesFilterParams;

	pesFilterParams.pid     = pes_pid;
	pesFilterParams.input   = DMX_IN_FRONTEND;
	pesFilterParams.output  = DMX_OUT_TS_TAP;
        pesFilterParams.pes_type = DMX_PES_OTHER;
	pesFilterParams.flags   = secflags;

	if (ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParams) < 0)  
		perror("DMX SET PES FILTER:");
}




void DVB::AddECM(Channel *chan, uint8_t *data, int length)
{
	int i;
	ecm_t *ecm = &chan->ecm;
	uint16_t sysid = chan->casystem;
	uint16_t pid = chan->capid;

        if (ecm->num>=MAX_ECM)
                return;
        for (i=0; i< ecm->num; i++)
                if ((ecm->sysid[i]==sysid) &&
                    (ecm->pid[i]==pid))
                        return ;
        ecm->sysid[ecm->num]=sysid;
        ecm->pid[ecm->num]=pid;
	if (length <= MAX_ECM_DESC){
		ecm->length[ecm->num] = (uint16_t)length;
		memcpy((char *)(ecm->data)+(ecm->num*MAX_ECM_DESC), 
		       (char *)data, length);
	}
        ecm->num++;
}

int DVB::check_ecm(Channel *chan)
{
	int found = 0;
	uint16_t prog_pid = 0;

	uint8_t buf[MAXSECSIZE], msec=0, sec=0;
	
	if (no_open) return -1;

	time_t count = time(0)+10;
	while (sec<=msec && !found && count > time(0)) {
		if (GetSection(buf, 0, 0, sec, msec)>0 && !buf[0]){
			sec++;
			found = parse_pat(chan,buf);
		}
	}
	if (!found) return -1;
	prog_pid = found;
	sec = 0;
	msec = 0;
	
	while (sec<=msec && count > time(0)) {
		if (GetSection(buf,prog_pid, 2, sec, msec)>0){
			sec++;
			parse_pmt(chan, buf);
			if(count < time(0)) break;
		}
	}

	return 0;
}


struct in_addr getaddress (const char *name)
{
	struct in_addr in;
	struct hostent *hp = gethostbyname (name);

	//fprintf(stderr,"Looking up host %s\n", name);
	
	
	if (hp)
		memcpy (&in.s_addr, hp->h_addr_list[0], sizeof (in.s_addr));
	else
	{
		fprintf(stderr, "couldn't find address of %s\n", name);
		exit (1);
	}
	
	return in;
}



int tcp_client_connect(const char *hostname, int sckt)
{
  
	int sock;
	struct sockaddr_in name;
	int dummy=-1;
	name.sin_family = AF_INET;
	name.sin_port = htons (sckt);
  
	name.sin_addr = getaddress(hostname);
  
	do {
		if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
			perror("socket");
			exit(1);
		}
		
		//fprintf(stderr,"Trying to connect...\n");

		if((dummy = connect(sock, (struct sockaddr *)&name, 
				    sizeof(name))) == ECONNREFUSED){
			perror("connect");
			exit(1);
		}

		if(dummy){
			sleep(1);
			close(sock);
		}
	} while(dummy);
	
//	fprintf(stderr,"Connection established.\n");
	
	return sock;
}

int udp_client_connect(const char *filename)
{
  
	int sock;
	struct sockaddr_un name;
	int dummy=-1;
	name.sun_family = AF_UNIX;
	snprintf(name.sun_path, 108, "%s", filename);
  
	do {
		if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1){
			perror("socket");
			exit(1);
		}
		
		fprintf(stderr,"Trying to connect...\n");

		if((dummy = connect(sock, (struct sockaddr *)&name, 
				    sizeof(name))) == ECONNREFUSED){
			perror("connect");
			exit(1);
		}

		if(dummy){
			sleep(1);
			close(sock);
		}
	} while(dummy);
	
	fprintf(stderr,"Connection established.\n");
	
	return sock;
}

void client_send_msg(int fd, uint8_t *msg, int size)
{
	int sent = send(fd,msg,size,0);

	if(sent == -1)
	{
		perror("send");
		exit(1);
	}
//	fprintf(stderr,"%d bytes sent.\n",sent);
}

int chck_frontend (int fefd, frontend_stat *festat)
{
	fe_status_t status;
	int i;
	uint16_t snr, signal;
	uint32_t ber, uncorrected_blocks;

	for (i = 0; i < 3; i++) {
		usleep (300000);
		if (ioctl(fefd, FE_READ_STATUS, &status) == -1) {
			perror("FE_READ_STATUS failed");
			return 0;
		}

		if (ioctl(fefd, FE_READ_SIGNAL_STRENGTH, &signal) == -1)
			signal = 0;
		if (ioctl(fefd, FE_READ_SNR, &snr) == -1)
			snr = 0;
		if (ioctl(fefd, FE_READ_BER, &ber) == -1)
			ber = 0;
		if (ioctl(fefd, FE_READ_UNCORRECTED_BLOCKS, 
			  &uncorrected_blocks) == -1)
			uncorrected_blocks = 0;

			
		if (status & FE_HAS_LOCK) return 1;
	}	
	return 0;
}


#define PAT_SCAN  0x01
#define PMT_SCAN  0x02
#define SDT_SCAN  0x04
#define EIT_SCAN  0x08


#define SCANNING  0x20
#define SET_AGAIN 0x40
#define SCAN_DONE 0x80
#define MAX_PIDS 100

int DVB::scan_tp(uint16_t tpid, uint16_t satid, int timeout, int verbose)
{
	int seclen=0;
	uint8_t section, sectionnum=0xff;
	uint8_t buf[MAXSECSIZE];
	struct pollfd pfd[MAX_PIDS];
	uint16_t pids[MAX_PIDS];
	uint16_t pnr[MAX_PIDS];
	uint8_t pid_state[MAX_PIDS];
	int filter_fds[MAX_PIDS];
	uint8_t sec[MAX_PIDS];
	uint8_t secn[MAX_PIDS];
	uint8_t maxsec[MAX_PIDS]; 
	time_t timeo[MAX_PIDS];
	int active = 0;
	int pidf = 0;
	uint16_t tsid;
	int oldcf=0;
	int eitn=0;

	if (timeout < 0) timeout = 5000;
	for (int i=0; i< num[CHAN]; i++)
		if (chans[i].tpid == tpid && chans[i].satid == satid)
			oldcf++;

	memset(pids, 0, MAX_PIDS*sizeof(uint16_t));
	memset(sec, 0, MAX_PIDS*sizeof(uint8_t));
	memset(secn, 0, MAX_PIDS*sizeof(uint8_t));
	memset(maxsec, 0, MAX_PIDS*sizeof(uint8_t));
	memset(pid_state, 0, MAX_PIDS*sizeof(uint8_t));
	memset(filter_fds, -1, MAX_PIDS*sizeof(int));
	for (int i=0; i<MAX_PIDS; i++){
		pfd[i].fd = -1;
		pfd[i].events = 0;
	}

	pids[0] = 0;
	sec[0] = 0x00;
	pid_state[0] |= PAT_SCAN | SET_AGAIN;

	pids[1] = 0x11;
	sec[1] = 0x42;
	pid_state[1] |= SDT_SCAN | SET_AGAIN;

	pidf=2;

	
	while(active || (pid_state[0] & SET_AGAIN)){
		int np=0;
		if (active && !(np = poll(pfd, active, timeout))){
			cerr << "TIMEOUT" << endl;
			for (int i=0; i< pidf; i++){
				if (pid_state[i] & SCANNING){
					if (verbose)
						cerr << "Still scanning pid " << pids[i] << endl;
					close (filter_fds[i]);
				}
			}
			break;
		} 
		for (int i=0; i< MAX_PIDS; i++){
			if ((pid_state[i] & SCANNING) && timeo[i] < time(0)){
				close (filter_fds[i]);
				secn[i]++;
				pfd[i].events = 0;
				if (secn[i] >= maxsec[i]){
					active--;
					if (verbose){
						if (pid_state[i] & PAT_SCAN)
							cerr << "  Stop PAT timeout" << endl;
						if (pid_state[i] & SDT_SCAN)
							cerr << "  Stop SDT timeout" << endl;
						if (pid_state[i] & EIT_SCAN)
							cerr << "  Stop EIT timeout" << endl;
						if (pid_state[i] & PMT_SCAN)
							cerr << "  Stop PMT 0x" << pids[i] 
							     << " timeout"  << endl;
					}
					pid_state[i] = SCAN_DONE;
				} 
			}
			
			if (np && (pid_state[i] & SCANNING) && (pfd[i].events & POLLIN)){
				seclen=0;
			
				if (read(filter_fds[i], buf, MAXSECSIZE) < 3)
					continue;
				seclen |= ((buf[1] & 0x03) << 8); 
				seclen |= (buf[2] & 0xFF);
				seclen += 3;
				section = buf[0];
				sectionnum = buf[6];
				if (secn[i] != sectionnum) continue;
	
				maxsec[i] = buf[7];
				secn[i]++;

				switch(pid_state[i] & 0x0F){
				case PAT_SCAN:
				{
					int c = 8;
					seclen -= 12;
					while (seclen>0){ 
						uint16_t npid= (buf[c] << 8)|buf[c+1];
						
						if (npid){
							pnr[pidf] = npid;

							pid_state[pidf] |= SET_AGAIN; 
							pids[pidf] = get_pid(buf+c+2);
							pid_state[pidf] |= PMT_SCAN;
							sec[pidf] = 0x02;
							pidf++;
						}
						seclen -= 4;
						c += 4;
					}
					if (secn[i] > maxsec[i]){
						pids[pidf] = 0x12;
						sec[pidf] = 0x4e;
						pid_state[pidf] |= (EIT_SCAN | SET_AGAIN);
						pidf++;
					}
					
					break;
				}

				case PMT_SCAN:
				{
					Channel chan;
					
					if (buf[0]!=0x02) break;
					chan.pnr = pnr[i];
					chan.satid = satid;
					chan.tpid = tpid;
					int j = AddChannel(chan);
					parse_pmt(&chans[j], buf);
					break;
				}

				case SDT_SCAN:
				{
					Channel *schan;
					int c = 11;
					uint16_t ilen=0;
					
					tsid = (buf[3]<<8)|buf[4+1];
					for (int t=0; t< num[TRANS]; t++){
						if (tps[t].id == tpid && 
						    tps[t].satid == satid){
							tps[t].tsid = tsid;
						}
					}

					while (c < seclen-4){
						uint16_t npnr;
						npnr = (buf[c]<<8)|buf[c+1];
						if(npnr){
							Channel chan;
							int found=0;
						
							chan.satid = satid;
							chan.tpid = tpid;
							chan.pnr = npnr;
							schan = &chan;
							for (int j = 0; j <num[CHAN]; j++){
								if (chans[j].pnr == npnr &&
								    chans[j].tpid == tpid && 
								    chans[j].satid == satid){
									schan = &chans[j];
									found=1;
									break;
								}
							}
 
							schan->has_eit = -1;
							schan->pres_follow = -1;
							if (buf[c+2] & 0x02) schan->has_eit = 0;
							if (buf[c+2] & 0x01) schan->pres_follow = 0;
							c+=3;
							schan->type=(buf[c]&0x10)>>4;
							ilen=((buf[c]&0x0f)<<8)|buf[c+1];
							c+=2;
							c+=parse_descriptor(schan, &buf[c], ilen,(verbose>1 ? 0:0));
							if (!found && strlen(schan->name))
								AddChannel(chan);
						}
					}
					break;
				}
				
				case EIT_SCAN:
				{
					int c = 14;
					uint16_t ilen=0;
					
					//show_buf(buf, seclen);
					while (c < seclen-4){
						c += 10;
						ilen=((buf[c]&0x0f)<<8)
							|buf[c+1];
						c+=2;
						
						c+=parse_descriptor(NULL, 
								    &buf[c], 
								    ilen,(verbose>1 ? 1:0));
					}
					break;
				}
				
				default:
					break;
				}

				if (secn[i] > maxsec[i]){
					pfd[i].events = 0;
					close (filter_fds[i]);
					active--;
					if (verbose>1){
						if (pid_state[i] & PAT_SCAN)
							cerr << "  Stop PAT" << endl;
						if (pid_state[i] & SDT_SCAN)
							cerr << "  Stop SDT" << endl;


						if (pid_state[i] & EIT_SCAN)
							cerr << "  Stop EIT" << endl;
						if (pid_state[i] & PMT_SCAN)
							cerr << "  Stop PMT 0x" << pids[i] << endl;
					}
					pid_state[i] = SCAN_DONE;
				} 
				
			
			}
			if (pidf>=MAX_PIDS){
				cerr << "MAX_PIDS too small" << endl;
				exit(1);
			}

			if (pid_state[i] & SET_AGAIN){
				if ((filter_fds[i] = 
				     SetFilter(pids[i], (sec[i]<<8)|0x00ff,O_NONBLOCK))==0xffff){
					pid_state[i] |= SET_AGAIN; 
				} else {
					if (verbose>1){
						if (pid_state[i] & PAT_SCAN)
							cerr << "  Start PAT scan 0x";
						if (pid_state[i] & SDT_SCAN)
							cerr << "  Start SDT scan 0x";
						if (pid_state[i] & EIT_SCAN)
							cerr << "  Start EIT scan 0x";
						if (pid_state[i] & PMT_SCAN)
							cerr << "  Start PMT scan 0x";

						cerr << hex << pids[i] << endl;
					}
					active++;
					pfd[i].fd = filter_fds[i];
					pfd[i].events = POLLIN;
					pid_state[i] &= ~SET_AGAIN;
					pid_state[i] |= SCANNING;
					timeo[i] = time(0)+4;

					if (eitn < 3){
						eitn++;
						pids[pidf] = 0x12;
						sec[pidf] = 0x4e;
						pid_state[pidf] |= (EIT_SCAN | SET_AGAIN);
						pidf++;
					}

				}
			}
		}
	}

	int cf=0;
	for (int i=0; i< num[CHAN]; i++){
		if (chans[i].tpid == tpid && chans[i].satid == satid){
			cf++;
/*
			int c = 0;

			if (chans[i].pnr)
				if (chans[i].vpid == NOPID)
					while (chans[i].apidnum == 0 && c<10) {
						check_pids(&chans[i]);
						c++;
					}
*/
			if (verbose) cerr << chans[i];
		}
	}
	for (int i=0; i<MAX_PIDS; i++){
		close(filter_fds[i]);
	}

	return cf-oldcf;
}


void DVB::scan_multi_eit(int verbose)
{
	int seclen=0;
	uint8_t section, sectionnum=0xff;
	uint8_t buf[MAXSECSIZE];
	struct pollfd pfd;
	uint16_t pid=0;
//	uint16_t pnr=NOPID;
	int filter_fd=-1;
	uint8_t sec=0;
	uint8_t secn=0;
	uint8_t maxsec=0; 
	time_t timeo=0;
//	uint16_t tsid;
	int timeout = 5000;
	int done = 0;

	pfd.fd = -1;
	pfd.events = 0;

	pid = 0x12;
	sec = 0x4e;

	cerr << "Scanning EIT" << endl;
	
	if ((filter_fd = SetFilter(pid, (sec<<8)|0x00ff,O_NONBLOCK))==0xffff){
		cerr << "Error setting EIT filter" << endl;
		return;
	} else {
		if (verbose>1){
			cerr << "  Start EIT scan 0x";
			cerr << hex << pid << endl;
		}
		pfd.fd = filter_fd;
		pfd.events = POLLIN;
		timeo = time(0)+4;
	}
					
	int np=0;
	while(!done){
		if (!(np = poll(&pfd, 1, timeout))){
			cerr << "TIMEOUT" << endl;
			break;
		} 
		if (timeo < time(0)){
			secn++;
			if (secn >= maxsec){
				done=1;
				if (verbose){
					cerr << "  Stop EIT timeout" << endl;
				}
			} 
		}
		if (np && (pfd.events & POLLIN)){
			seclen=0;
			
			cerr << "found section" << endl;
			if (read(filter_fd, buf, MAXSECSIZE) < 3)
				continue;
			seclen |= ((buf[1] & 0x03) << 8); 
			seclen |= (buf[2] & 0xFF);
			seclen += 3;
			section = buf[0];
			sectionnum = buf[6];
			if (secn != sectionnum) continue;
			
			maxsec = buf[7];
			secn++;
			
			int c = 14;
			uint16_t ilen=0;
			
			//show_buf(buf, seclen);
			while (c < seclen-4){
				c += 10;
				ilen=((buf[c]&0x0f)<<8)
					|buf[c+1];
				c+=2;
				c+=parse_descriptor(NULL, &buf[c], ilen, verbose);
			}
			
			if (secn > maxsec){
				pfd.events = 0;
				close (filter_fd);
				done=1;
				if (verbose>1){
					cerr << "  Stop EIT" << endl;
				}
			} 
		}
	}
}




int DVB::scan_TP(uint16_t tpid, uint16_t satid, int timeout, int verbose)
{
	if (no_open) return -1;

	if (verbose){
		cerr << "Setting Transponder 0x" << HEX(4) 
		     << tpid << "  ";
		for (int i = 0; i < num[TRANS]; i++){
			if (tps[i].id == tpid){
				cerr << dec << tps[i].freq/1000
				     << (tps[i].pol ? "H":"V")
				     << " " << tps[i].srate/1000
				     << endl;
				break;
			}
		}
	}

	get_front();
	if (SetTP(tpid, satid) < 0) return -1;
	if (set_front() < 0)  return -1;
	if (verbose) cerr << endl << "Starting transponder scan" << endl;
	return scan_tp(tpid, satid, timeout, verbose);
}

int DVB::scan_current(int timeout, int verbose)
{
	return scan_tp(1000, 1000, timeout, verbose);
}

uint8_t hamtab[256] = {
  0x01, 0xff, 0x81, 0x01, 0xff, 0x00, 0x01, 0xff,
  0xff, 0x02, 0x01, 0xff, 0x0a, 0xff, 0xff, 0x07,
  0xff, 0x00, 0x01, 0xff, 0x00, 0x80, 0xff, 0x00,
  0x06, 0xff, 0xff, 0x0b, 0xff, 0x00, 0x03, 0xff,
  0xff, 0x0c, 0x01, 0xff, 0x04, 0xff, 0xff, 0x07,
  0x06, 0xff, 0xff, 0x07, 0xff, 0x07, 0x07, 0x87,
  0x06, 0xff, 0xff, 0x05, 0xff, 0x00, 0x0d, 0xff,
  0x86, 0x06, 0x06, 0xff, 0x06, 0xff, 0xff, 0x07,
  0xff, 0x02, 0x01, 0xff, 0x04, 0xff, 0xff, 0x09,
  0x02, 0x82, 0xff, 0x02, 0xff, 0x02, 0x03, 0xff,
  0x08, 0xff, 0xff, 0x05, 0xff, 0x00, 0x03, 0xff,
  0xff, 0x02, 0x03, 0xff, 0x03, 0xff, 0x83, 0x03,
  0x04, 0xff, 0xff, 0x05, 0x84, 0x04, 0x04, 0xff,
  0xff, 0x02, 0x0f, 0xff, 0x04, 0xff, 0xff, 0x07,
  0xff, 0x05, 0x05, 0x85, 0x04, 0xff, 0xff, 0x05,
  0x06, 0xff, 0xff, 0x05, 0xff, 0x0e, 0x03, 0xff,
  0xff, 0x0c, 0x01, 0xff, 0x0a, 0xff, 0xff, 0x09,
  0x0a, 0xff, 0xff, 0x0b, 0x8a, 0x0a, 0x0a, 0xff,
  0x08, 0xff, 0xff, 0x0b, 0xff, 0x00, 0x0d, 0xff,
  0xff, 0x0b, 0x0b, 0x8b, 0x0a, 0xff, 0xff, 0x0b,
  0x0c, 0x8c, 0xff, 0x0c, 0xff, 0x0c, 0x0d, 0xff,
  0xff, 0x0c, 0x0f, 0xff, 0x0a, 0xff, 0xff, 0x07,
  0xff, 0x0c, 0x0d, 0xff, 0x0d, 0xff, 0x8d, 0x0d,
  0x06, 0xff, 0xff, 0x0b, 0xff, 0x0e, 0x0d, 0xff,
  0x08, 0xff, 0xff, 0x09, 0xff, 0x09, 0x09, 0x89,
  0xff, 0x02, 0x0f, 0xff, 0x0a, 0xff, 0xff, 0x09,
  0x88, 0x08, 0x08, 0xff, 0x08, 0xff, 0xff, 0x09,
  0x08, 0xff, 0xff, 0x0b, 0xff, 0x0e, 0x03, 0xff,
  0xff, 0x0c, 0x0f, 0xff, 0x04, 0xff, 0xff, 0x09,
  0x0f, 0xff, 0x8f, 0x0f, 0xff, 0x0e, 0x0f, 0xff,
  0x08, 0xff, 0xff, 0x05, 0xff, 0x0e, 0x0d, 0xff,
  0xff, 0x0e, 0x0f, 0xff, 0x0e, 0x8e, 0xff, 0x0e,
};

uint8_t invtab[256] = {
  0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
  0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
  0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
  0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
  0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
  0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
  0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
  0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
  0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
  0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
  0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
  0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
  0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
  0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
  0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
  0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
  0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
  0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
  0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
  0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
  0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
  0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
  0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
  0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
  0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
  0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
  0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
  0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
  0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
  0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
  0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
  0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};


uint8_t deham(uint8_t x, uint8_t y)
{
	return (hamtab[y]<<4)|(0x0f&hamtab[x]);	
}


static int create_dir(const char *path)
{
        int retval;
        char *bufp, *new_path;
        struct stat sb;

        retval=0;
        if(path && *path) {
                new_path = strdup(path);
                for(bufp = new_path+1; *bufp; ++bufp) {
                        if(*bufp == '/') {
                                *bufp = 0;
                                if(stat(new_path,&sb)<0) {
                                        retval = mkdir(new_path, 0755);
                                }
                                *bufp = '/';
                        }
                }
                free(new_path);
        }
        return retval;
}

void DVB::add_vtx_line(magazin_t *mag, int line, uint8_t *data, int pnr)
{
        uint8_t c=0;
        FILE *fd;
        char fname[1024];
        uint8_t buf;

        if(!line) {
                mag->valid = 1;
                memset(mag->pagebuf, ' ', 25*40);
                mag->pnum = deham(data[0], data[1]);
                if(mag->pnum == 0xff) return;
                mag->flags = deham(data[2],data[3])&0x80;
                mag->flags |= (c&0x40)|((c>>2)&0x20);
                c = deham(data[6],data[7]);
                mag->flags |= ((c<<4)&0x10)|((c<<2)&0x08)|(c&0x04)
			|((c>>1)&0x02)|((c>>4)&0x01);
                mag->lang = ((c>>5) & 0x07);
                mag->sub = (deham(data[4],data[5])<<8)|
			(deham(data[2],data[3])&0x3f7f);
        }

        if(mag->valid) {
                if (line <= 23)
                        memcpy(mag->pagebuf+40*line,data,40);
                if (line==23) {
                        int pagenumber=(mag->magn*100) + ((mag->pnum>>4)*10) 
				+ (mag->pnum & 0x0f);
                        snprintf(fname,1024,"%s/%d_%d_%c_%d/",vtxdir,
				 transponder_freq,
				 transponder_srate,
				 transponder_pol,
				 pnr);
                        create_dir(fname);
                        snprintf(fname,1024,"%s/%d_%d_%c_%d/%d_%d.vtx",vtxdir,
				 transponder_freq,
				 transponder_srate,
				 transponder_pol,
				 pnr,pagenumber,
				 mag->sub&0xff);
                        if ((fd=fopen(fname,"w"))) {
				fwrite("VTXV4",1,5,fd);
                                buf = 0x01;
                                fwrite(&buf,1,1,fd);
                                buf = mag->magn;
                                fwrite(&buf,1,1,fd);
                                buf = mag->pnum;
                                fwrite(&buf,1,1,fd);
                                buf = 0x00;
                                fwrite(&buf,1,1,fd);    
                                fwrite(&buf,1,1,fd);
                                fwrite(&buf,1,1,fd);
                                fwrite(&buf,1,1,fd);
                                fwrite(mag->pagebuf,1,24*40,fd);
                                fclose(fd);
                        }
                        mag->valid=0;
                }
        }
}


