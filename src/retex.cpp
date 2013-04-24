//============================================================================
// Name        : retex.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

//#include <iostream>
#include <syslog.h>
#include "RetexAdapter.h"
#include "CmdLineParser.h"
//using namespace std;

int main(int argc, char* argv[]) {
	CmdLineParser* pCmdLineParser = new CmdLineParser(argc, argv);

	if (pCmdLineParser != NULL) {
		//check syntax and build command structure
		if (pCmdLineParser->Parse()) {
			RetexAdapter* pAdapter = new RetexAdapter(pCmdLineParser);
			// go into the loop which either run the daemon or exits after command line parameter processing
			if(pAdapter) {
				pAdapter->Run();
				delete pAdapter;
			}
		}
		delete pCmdLineParser;
	}
	return 0;
}
