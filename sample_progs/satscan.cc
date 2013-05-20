#include <iostream>
#include <getopt.h>
#include <DVB.hh>

enum {Q_YESNO, Q_INTEGER, Q_FLOAT};

int ask(const char *qe, int type, void *r)
{
        char ans[80];
        cerr << qe << " ?";
        switch(type){
        case Q_YESNO:
                cerr << " (Y/N) ";
                cin >> ans;
                if ( ans[0] == 'Y' ) return 1;
                if ( ans[0] == 'y' ) return 1;
                return 0;

        case Q_INTEGER:
        {
                int *i = (int *)r;
                *i = 0;
                cin >> *i;
        }
        return 0;

        case Q_FLOAT:
        {
                float *d = (float *)r;
                *d = 0;
                cin >> *d;
        }
                return 0;
        }
        return -1;
}

void usage()
{
	cerr << "usage: satscan [options] <dvbrcfile>" << endl
	     << "-h          print this help" << endl
	     << "-a <N>      use /dev/dvb/adaptorN" << endl
	     << "-q <d>      defaultdiseqc d" << endl
	     << "-v          verbose" << endl
	     << "-s          no tuning, check current TP" << endl
	     << "-n          no interaction" << endl
	     << "-i          check other inversion settings" << endl
	     << "-l          set lof1 of LNB, e.g.  9750000" << endl
	     << "-o          set lof2 of LNB, e.g. 10600000" << endl
	     << "-f          set slof of LNB, e.g. 11700000" << endl;
	exit(0);
}


int main(int argc, char **argv)
{
	int n=0;
	int adapter=0;
	int c;
	int v=1;
	int si=0;
	int dontask=0;
	int defaultdiseqc=0;
	int tryinv=0;
	unsigned int lof1=0;
	unsigned int lof2=0;
	unsigned int slof=0;

	if (argc < 2) usage();
        for (;;) {
                if (-1 == (c = getopt(argc, argv, "l:o:f:a:q:vsnih")))
                        break;
                switch (c) {
		case 'l':
			lof1 = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'o':
			lof2 = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'f':
			slof = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'a':
			adapter = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'q':
			defaultdiseqc = strtol(optarg, (char **) NULL, 0);
			break;
		case 'v':
			v=2;
			break;
		case 's':
			si=1;
			break;
		case 'n':
			dontask=1;
			break;
		case 'i':
			tryinv = 1;
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	if (optind<=0){
		cerr << "Enter Transponder list" << endl;
		exit(1);
	}

        ifstream con(argv[optind]);	
	DVB dvb(adapter);

	if (si){
		cerr << "Checking current transponder\n" << endl;
		dvb.search_in_TP(0,0,0,2);
		exit(0);
	}

	con >> dvb;

	if (dontask || ask("Clear all channels", Q_YESNO, NULL)){
		cerr << "Clearing channels" << endl;
		for (int i=0; i<dvb.num[CHAN]; i++) {
			dvb.chans[i].clearall();
		}
		dvb.num[CHAN]=0;
	}
	for (int s=0; s < dvb.num[SAT]; s++){
		ostringstream satq;
		ostringstream diseqc;
		ostringstream inver;
                uint16_t satid;

		satq << "Check " << dvb.sats[s].name << " satellite" << ends;
		if (!dontask && !ask(satq.str().data(), Q_YESNO, NULL)) continue;

		int lnb=-1;
		for (int i=0; i<dvb.num[LNB]; i++)
			if (dvb.sats[s].lnbid == dvb.lnbs[i].id){
				lnb=i;
				break;
			}
		if (lnb>=0){
			diseqc << "Use different Diseqc (" 
			       << dvb.lnbs[lnb].diseqcnr << ")" << ends;
			if (!dontask && ask(diseqc.str().data(), Q_YESNO, NULL)){
				ask("Enter diseqc Number",Q_INTEGER, 
				    &dvb.lnbs[lnb].diseqcnr);
				dvb.lnbs[lnb].id=dvb.lnbs[lnb].diseqcnr;
				dvb.sats[s].lnbid=dvb.lnbs[lnb].diseqcnr;
			} else if(dontask && defaultdiseqc) {
				dvb.lnbs[lnb].diseqcnr = defaultdiseqc;
				dvb.lnbs[lnb].id = defaultdiseqc;
				dvb.sats[s].lnbid = defaultdiseqc;
			}
			if (lof1) dvb.lnbs[lnb].lof1 = lof1;
			if (lof2) dvb.lnbs[lnb].lof2 = lof2;
			if (slof) dvb.lnbs[lnb].slof = slof;
		}
		satid = dvb.sats[s].id;
		for (int t=0; t < dvb.num[TRANS]; t++){
			if (dvb.tps[t].satid == satid){
				uint16_t tpid;
				int c=0;
				tpid = dvb.tps[t].id;
				if ((c = dvb.scan_TP(tpid,satid,-1,v))<0) 
					cerr << "Error in TP scan" <<endl;
				if (c>0){
					n+=c;
					cerr << " found " << dec 
					     <<  c << " channel(s)" 
					     << endl << endl;;
				} else {
					if (tryinv){
						if (dvb.tps[t].inversion 
						    == INVERSION_OFF)
							dvb.tps[t].inversion = 
								INVERSION_ON;
						else if (dvb.tps[t].inversion == 
							 INVERSION_ON)
							dvb.tps[t].inversion = 
								INVERSION_OFF;
						
						if ((c = dvb.scan_TP(tpid,satid,
								     -1,v))<0) 
							cerr << "Error in TP scan"
							     <<endl;
					}
					if (c>0){
						n+=c;
						cerr << " found " << dec 
						     <<  c << " channel(s)" 
						     << endl << endl;;
					}
					
				}

			}
		}
	}
		
	cerr << "Found " << dec << n << " new channels" << endl;
	
	cout << dvb;

	return (n > 0) ? 0 : 1;
}
