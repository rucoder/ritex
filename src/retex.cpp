//============================================================================
// Name        : retex.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
//============================================================================

#include <syslog.h>
#include "RetexAdapter.h"
#include "CmdLineParser.h"

int main(int argc, char* argv[]) {
	CmdLineParser* pCmdLineParser = new CmdLineParser(argc, argv);

	if (pCmdLineParser != NULL) {
		//check syntax and build command structure
		if (pCmdLineParser->Parse()) {
			Adapter* pAdapter = new RitexAdapter("Ritex", "v 0.1.7 alpha", "Ritex adapter", pCmdLineParser);
			// go into the loop which either run the daemon or exits after command line parameter processing
			if(pAdapter) {
				pAdapter->Run();
				delete pAdapter;
			}
			syslog(LOG_ERR, "[main()] EXIT");
		}
	}
	return 0;
}
