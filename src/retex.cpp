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

	syslog(LOG_ERR, "Starting ritex with %d arguments", argc - 1);
	syslog(LOG_ERR, "Command line:");
	for (int i = 1; i < argc; i++)
		syslog(LOG_ERR, "%s", argv[i]);

	CmdLineParser* pCmdLineParser = new CmdLineParser(argc, argv);

	if (pCmdLineParser != NULL) {
		//check syntax and build command structure
		if (pCmdLineParser->Parse()) {
			RetexAdapter* pAdapter = new RetexAdapter(pCmdLineParser);
			// go into the loop which either run the daemon or exits after command line parameter processing
			pAdapter->Run();
		}
	}
	return 0;
}
