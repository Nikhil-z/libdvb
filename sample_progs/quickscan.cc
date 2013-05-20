#include <iostream>
#include <getopt.h>
#include <DVB.hh>


int main(int argc, char **argv)
{
	int n=0;
	int adapter=0;
	int c;
	int v=1;

        for (;;) {
                if (-1 == (c = getopt(argc, argv, "a:v")))
                        break;
                switch (c) {
		case 'a':
			adapter = strtol(optarg,(char **) NULL, 0);
                        break;
		case 'v':
			v=2;
			break;
		}
	}

	DVB dvb(-1);
	dvb.adapter=adapter;


	if ((c = dvb.scan_current(-1,v))<0) 
		cerr << "Error in TP scan" <<endl;
	if (c>0){
		n+=c;
		cerr << " found " << dec <<  c << " channel(s)" 
		     << endl << endl;;
	}
}
