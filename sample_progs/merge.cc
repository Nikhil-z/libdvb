#include <iostream>
#include "DVB.hh"

int main(int argc, char **argv)
{
	int i;
	DVB dvbd(-1);
  

	for (i = 0 ; i < argc-1 ; i++){
		ifstream ifs(argv[i+1]);
		if (!ifs) {
			cerr << "Couldn't open " << argv[2] << endl;
			exit(1);
		}
		cerr << "Reading " << argv[i+1] << endl;
		ifs >> dvbd;
	}

	cout << dvbd;
}
