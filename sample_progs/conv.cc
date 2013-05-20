#include <iostream>
#include "DVB.hh"

int main(int argc, char **argv)
{
        ifstream con(argv[1]);	
	DVB dvbd(-1);
	con >> dvbd;

	//	dvbd.set_outtype(VDR_OUT);
	cout << dvbd;
}
