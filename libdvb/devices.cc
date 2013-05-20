#include "devices.hh"
#include <linux/dvb/frontend.h>


static char *feckeys[]={"NONE", "1/2", "2/3", "3/4", "4/5", "5/6", "6/7", 
			"7/8", "8/9", "AUTO", "0", "1", "2", "3", "4", "5", 
			"6", "7", "8", "9"};

static char *invkeys[]={"OFF", "off", "on", "ON", "AUTO", "auto"};
static fe_spectral_inversion_t invset[]={INVERSION_OFF, INVERSION_OFF, 
					       INVERSION_ON, INVERSION_ON,
					       INVERSION_AUTO, INVERSION_AUTO};

enum { TID=0, TNAME, TTYP, TFREQ, TPOL, TQAM, TSRATE, TFEC, TSAT, TONID, 
       TBANDWIDTH, TGUARD_INTERVAL, THIERARCHY, THP_RATE, TLP_RATE, 
       TMODULATION, TTRANSMISSION_MODE, TSID, TINVERSION};

static char *tpkeys[]={
        "ID", "NAME", "TYPE", "FREQ", "POL",
	"QAM", "SRATE", "FEC", "SATID", "ONID","BANDWIDTH", "GUARD_INTERVAL",
	"HIERARCHY", "HP_RATE", "LP_RATE", "MODULATION", "TRANSMISSION_MODE",
	"TSID","INVERSION",
	NULL
};

enum { SID=0, SNAME, SLNB, SROTOR, SFMIN, SFMAX};
static char *satkeys[]={
        "ID", "NAME", "LNBID", "ROTORID", "FMIN", "FMAX", NULL
};

enum {LID=0, LNAME, LTYP, LLOF1, LLOF2, LSLOF, LDIS, LROT, LDISNR};
static char *lnbkeys[]={
        "ID", "NAME", "TYPE", "LOF1", "LOF2", "SLOF",
	"DISEQCID", "ROTORID", "DISEQCNR", NULL
};

enum {CID=0, CNAME, CTYP, CVPID , CAPID, CPNR, CPCR, CTP, CTTP, CSID, 
      CSAT, CONID, CBOQ, CPNAME, CNNAME, CANAME, CAC3PID, CSUBPID};
static char *chkeys[]={
        "ID", "NAME", "TYPE", "VPID", "APID", "PNR", "PCRPID", "TPID",
	"TTPID", "SID", "SATID", "ONID", "BID", "PNAME", "NNAME","ANAME", "AC3PID", "SUBPID",  
	NULL
};

int findkey(char *name, char *keys[])
{
        int i=-1;
	char *key=keys[0];
	uint l;

	while ((key=keys[++i])) {
		l = strlen(name);
		if (strlen(key)>l) continue;
		if(!strncmp(name, key, l)){
		        return i;
		}
			
	}
	return -1;
}

void getname(char *name,istream &ins, char startc, char stopc)
{
        char cdummy[MAXNAM+2];
	uint8_t l;

	streampos p,p2;
	if (startc) ins.ignore(1000, startc);
	
	p=ins.tellg();
	ins.get(cdummy,MAXNAM+1); // get full channel name
	ins.seekg(p);
	ins.ignore(1000, stopc);
	p2=ins.tellg();
	if ( (l = p2-p-streampos(1)) > MAXNAM) l=MAXNAM;
	strncpy(name,cdummy,l);
	name[l]='\0';
}

ostream &operator<<(ostream &stream, Lnb &lnb) {
        stream << "LNB " << "ID " << hex << lnb.id;
	if (lnb.name[0]) 
	        stream << " NAME \"" << lnb.name << "\""; 
	stream << " TYPE " << dec << lnb.type << " "; 
	if (lnb.type == FE_QPSK){
		if (lnb.lof1)
			stream << " LOF1 " << dec << lnb.lof1; 
		if (lnb.lof2)
			stream << " LOF2 " << dec << lnb.lof2; 
		if (lnb.slof)
			stream << " SLOF " << dec << lnb.slof; 
		if (lnb.diseqcnr!=-1)
			stream << " DISEQCNR " << dec << lnb.diseqcnr; 
		if (lnb.diseqcid!=NOID)
			stream << " DISEQCID " << hex << lnb.diseqcid; 
		if (lnb.swiid!=NOID)
			stream << " SWITCHID " << hex << lnb.swiid; 
	}
	stream << "\n";
	return stream;
};

ostream &operator<<(ostream &stream, Sat &sat) {
	stream << "  SAT " << "ID " << hex <<  sat.id;
	if (sat.name[0]) 
  	        stream << " NAME \"" << sat.name << "\""; 
	stream << " LNBID " << hex << sat.lnbid; 
	stream << " FMIN " << dec << sat.fmin; 
	stream << " FMAX " << dec << sat.fmax; 
	if (sat.rotorid!=NOID)
	        stream << " ROTORID " << hex << sat.rotorid; 
	stream << "\n";
	return stream;
};

ostream &operator<<(ostream &stream, Transponder &tp) {
        stream << "    TRANSPONDER " << "ID " << HEX(4) << tp.id;
	if (tp.tsid!=NOID)
	  stream << " TSID " << HEX(4) << tp.tsid; 
	if (tp.satid!=NOID)
	  stream << " SATID " << HEX(4) << tp.satid; 
	stream << " TYPE " << hex << tp.type;
	if (tp.name[0]) 
  	        stream << " NAME \"" << tp.name << "\""; 
	stream << " FREQ " << dec << tp.freq;

	if (tp.type == FE_QPSK)
	        stream << " POL " << (tp.pol ? "H" : "V" ) ;
	if (tp.type == FE_QAM)
	        stream << " QAM " << dec << tp.qam;        
	
	if (tp.type == FE_QPSK || tp.type == FE_QAM) {
                stream << " SRATE " << dec << tp.srate;
                stream << " FEC " << feckeys[tp.fec];
        }
	
	if (tp.type == FE_OFDM){
	        stream << " BANDWIDTH " << dec << tp.band;
	        stream << " HP_RATE " << dec << tp.hp_rate;
	        stream << " LP_RATE " << dec << tp.lp_rate;
	        stream << " MODULATION " << dec << tp.mod;
	        stream << " TRANSMISSION_MODE " << dec << tp.transmode;
	        stream << " GUARD_INTERVAL " << dec << tp.guard;
	        stream << " HIERARCHY " << dec << tp.hierarchy;
	}
	switch(tp.inversion){
	case INVERSION_OFF:
		stream << " INVERSION off";
		break;
	case INVERSION_ON:
		stream << " INVERSION on";
		break;
	case INVERSION_AUTO:
		stream << " INVERSION auto";
		break;
	}
	stream << "\n";
	return stream;
};

ostream &operator<<(ostream &stream, Channel &ch) {
        stream << "      CHANNEL";
	stream << " ID " << hex << ch.id;
	if (ch.name[0]) 
  	        stream << " NAME \"" << ch.name << "\""; 
	if (ch.prov_name[0]) 
  	        stream << " PNAME \"" << ch.prov_name << "\""; 
	if (ch.net_name[0]) 
  	        stream << " NNAME \"" << ch.net_name << "\""; 
	stream << " SATID " << hex << ch.satid;
	stream << " TPID " << hex << ch.tpid;
	stream << " SID " << hex << ch.pnr;
	stream << " TYPE " << hex << ch.type;
	if (ch.vpid!=NOPID)
	        stream << " VPID " << hex << ch.vpid;
	for (int i=0; i<ch.apidnum; i++){
	        stream << " APID " << hex << ch.apids[i];
		if (strlen(&ch.apids_name[i*4]) < 3 &&
		    strlen(&ch.apids_name[i*4]) > 0){
		       stream << " ANAME \"" 
			      << (char *)(ch.apids_name+i*4) << "\"";
		}
	}
	if (ch.ttpid && ch.ttpid!=NOPID)  // don't know where pid 0 comes from
	        stream << " TTPID " << hex << ch.ttpid;
	if (ch.pmtpid!=NOPID)
	        stream << " PMTPID " << hex << ch.pmtpid;
	if (ch.pcrpid!=NOPID)
	        stream << " PCRPID " << hex << ch.pcrpid;
	if (ch.ac3pid!=NOPID)
	        stream << " AC3PID " << hex << ch.ac3pid;

	if (ch.subpid!=NOPID)
	        stream << " SUBPID " << hex << ch.subpid;

	if (ch.onid!=NOID)
	        stream << " ONID " << hex << ch.onid;
	if (ch.bid!=NOID)
	        stream << " BID " << hex << ch.bid;

	stream << "\n";
	return stream;
};


istream &operator>>(istream &ins, Sat &x){
  int n;
  char keybuf[MAXNAM];
    
  while(!ins.eof()) {
    streampos pos = ins.tellg();
    ins.width(MAXNAM);
    ins >> keybuf;
    n=findkey(keybuf, satkeys);
    if (n<0) {
      ins.seekg(pos);
      break;
    }
    switch (n) {
    case SID: 
      ins >> hex >> x.id;
      break;
    case SNAME: 
      getname(x.name,ins);
      break;
    case SLNB: 
      ins >> hex >> x.lnbid;
      break;
    case SROTOR: 
      ins >> hex >> x.rotorid;
      break;
    case SFMIN: 
      ins >> dec >> x.fmin;
      break;
    case SFMAX: 
      ins >> dec >> x.fmax;
      break;
    } 
  }
  if (x.id==NOID || x.lnbid==NOID || x.fmin==0 || x.fmax==0 ){
    cerr << "Error: Not enough information for SAT" << endl;
    exit(1);
  }
  return ins;
}

istream &operator>>(istream &ins, Lnb &x){
  int n;
  char keybuf[MAXNAM];


  while(!ins.eof()) {
    streampos pos = ins.tellg();
    ins.width(MAXNAM);
    ins >> keybuf;
    n=findkey(keybuf, lnbkeys);
    
    if (n<0) {
      ins.seekg(pos);
      break;
    } else {
      switch (n) {
      case LID: 
	ins >> hex >> x.id;
	break;
      case LNAME:
	getname(x.name,ins);
	break;
      case LTYP: 
	ins >> x.type;
	break;
      case LLOF1: 
	ins >> dec >> x.lof1;
	break;
      case LLOF2: 
	ins >> dec >> x.lof2;
	break;
      case LSLOF: 
	ins >> dec >> x.slof;
	break;
      case LDIS: 
	ins >> hex >> x.diseqcid;
	break;
      case LDISNR: 
	ins >> dec >> x.diseqcnr;
	break;
      }
    }
  }
  if (x.id==NOID || x.type==-1){
    cerr << "Error: Not enough information for LNB" << endl;
    exit(1);
  }
  return ins;
}


istream &operator>>(istream &ins, Transponder &x){
  int n;
  char keybuf[MAXNAM];
  
  x.fec = (int) FEC_AUTO;
  x.inversion = INVERSION_OFF;
  while(!ins.eof()) {
	  streampos pos = ins.tellg();
	  ins.width(MAXNAM);
	  ins >> keybuf;
	  n=findkey(keybuf, tpkeys);
	  if (n<0) {
		  ins.seekg(pos);
		  break;
	  }
	  switch (n) {
	  case TID: 
		  ins >> hex >> x.id;
		  break;
	  case TSID: 
		  ins >> hex >> x.tsid;
		  break;
	  case TNAME:
		  getname(x.name,ins);
		  break;
	  case TTYP:
		  ins >> dec >> x.type;
		  break;
	  case TFREQ:
		  ins >> dec >> x.freq;
		  break;
	  case TPOL:
	  {
		  ins.width(MAXNAM);
		  ins >> keybuf;
		  if (keybuf[0]=='H') {
			  x.pol=1;
			  break;
		  }
		  if (keybuf[0]=='V') {
			  x.pol=0;
			  break;
		  }
		  ins.seekg(pos);
		  return ins;
		  if (!x.type) x.type = FE_QPSK;
	  }
	  break;
	  case TQAM:
		  ins >> x.qam;
		  if (!x.type) x.type = FE_QAM;
		  break;
	  case TSRATE:
		  ins >> dec >> x.srate;
		  break;
	  case TONID:
		  ins >> hex >> x.onid;
		  break;
	  case TFEC:
		  ins.width(MAXNAM);
		  ins >> keybuf;
		  x.fec = findkey(keybuf, feckeys);
		  if (x.fec > int(FEC_AUTO) )
	                  x.fec -= (int(FEC_AUTO) + 1);
	          if ((x.fec < 0) || (x.fec > int(FEC_AUTO)))
	                  x.fec = FEC_AUTO;
		  break;
	  case TSAT:
		  ins >> hex >> x.satid;
		  break;
	  case TBANDWIDTH: 
		  ins >> dec >> x.band;
		  if (!x.type) x.type = FE_OFDM;
		  break;
	  case THP_RATE: 
		  ins.width(MAXNAM);
		  ins >> keybuf;
		  x.hp_rate = findkey(keybuf, feckeys);
		  if (x.hp_rate > int(FEC_AUTO) )
	                  x.hp_rate -= (int(FEC_AUTO) + 1);
	          if ((x.hp_rate < 0) || (x.hp_rate > int(FEC_AUTO)))
	                  x.hp_rate = FEC_AUTO;
		  break;
	  case TLP_RATE: 
		  ins.width(MAXNAM);
		  ins >> keybuf;
		  x.lp_rate = findkey(keybuf, feckeys);
		  if (x.lp_rate > int(FEC_AUTO) )
	                  x.lp_rate -= (int(FEC_AUTO) + 1);
	          if ((x.lp_rate < 0) || (x.lp_rate > int(FEC_AUTO)))
	                  x.lp_rate = FEC_AUTO;
		  break;
	  case TMODULATION: 
		  ins >> dec >> x.mod;
		  break;
	  case TTRANSMISSION_MODE: 
		  ins >> dec >> x.transmode;
		  break;
	  case TGUARD_INTERVAL: 
		  ins >> dec >> x.guard;
		  break;
	  case THIERARCHY: 
		  ins >> dec >> x.hierarchy;
		  break;

	  case TINVERSION: {
		  int inv;
		  ins.width(MAXNAM);
		  ins >> keybuf;
		  inv = findkey(keybuf, invkeys);
		  x.inversion = invset[inv]; 
		  break;
	  }
	  } 
  }
  if (x.id==NOID || x.freq==0 ){
	  cerr << "Error: Not enough information for TRANSPONDER" << endl;
	  exit(1);
  }
  return ins;
}


istream &operator>>(istream &ins, Channel &x){
  int n;
  char keybuf[MAXNAM];
  
  while(!ins.eof()) {
    streampos pos = ins.tellg();
    ins.width(MAXNAM);
    ins >> keybuf;

    n=findkey(keybuf, chkeys);
    if (n<0) {
      ins.seekg(pos);
      break;
    }
    switch (n) {
    case CID: 
      ins >> hex >> x.id;
      break;
    case CSAT: 
      ins >> hex >> x.satid;
      break;
    case CONID: 
      ins >> hex >> x.onid;
      break;
    case CBOQ: 
      ins >> hex >> x.bid;
      break;
    case CNAME:
      getname(x.name,ins);
      break;
    case CPNAME:
      getname(x.prov_name,ins);
      break;
    case CNNAME:
      getname(x.net_name,ins);
      break;
    case CTYP:
      ins >> dec >> x.type;
      break;
    case CVPID:
      ins >> hex >> x.vpid;
      break;
    case CTTP:
      ins >> hex >> x.ttpid;
      break;
    case CAC3PID:
      ins >> hex >> x.ac3pid;
      break;
    case CSUBPID:
      ins >> hex >> x.subpid;
      break;
    case CAPID:
      if (x.apidnum>=MAXAPIDS) 
	break;
      ins >> hex >> x.apids[x.apidnum];
      x.apidnum++;
      break;
    case CANAME:
    {
	    char n[MAXNAM+1];
	    if (!x.apidnum) break;
	    getname(n,ins);
	    if (x.apidnum <= MAXAPIDS){
		    memset((char *)(x.apids_name+(x.apidnum-1)*4),0,4);
		    memcpy((char *)(x.apids_name+(x.apidnum-1)*4),n,3);
	    }
	    break;
    }
    case CPNR:
    case CSID:
      ins >> hex >> x.pnr;
      break;
    case CPCR:
      ins >> hex >> x.pcrpid;
      break;
    case CTP:
      ins >> hex >> x.tpid;
      break;
    };
  }
  if (x.id==NOID || x.type==-1 || x.tpid==NOPID || (x.pnr==NOPID && 
						    (x.vpid==NOPID|| 
						     x.apids[0]==NOPID)) 
						    
	      ){
    cerr << "Error: Not enough information for CHANNEL " << x << endl;
    exit(1);
  }
  return ins;
}

istream &operator>>(istream &ins, Bouquet &x){
  return ins;
}

istream &operator>>(istream &ins, DiSEqC &x){
        return ins;
}
istream &operator>>(istream &ins, Rotor &x){
        return ins;
}


