//============================================================================
// Name        : retex.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <syslog.h>
#include "RetexAdapter.h"
#include "CmdLineParser.h"

int main(int argc, char* argv[]) {
	CmdLineParser* pCmdLineParser = new CmdLineParser(argc, argv);

	if (pCmdLineParser != NULL) {
		//check syntax and build command structure
		if (pCmdLineParser->Parse()) {
			Adapter* pAdapter = new RitexAdapter("Ritex", "v 0.1 alpha", "Ritex adapter", pCmdLineParser);
			// go into the loop which either run the daemon or exits after command line parameter processing
			if(pAdapter) {
				pAdapter->Run();
				delete pAdapter;
			}
		}
	}
	return 0;
}
