//============================================================================
// Name        : retex.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
//============================================================================

#include <syslog.h>
#include "RetexAdapter.h"
#include "CmdLineParser.h"
#include "Log.h"
#include "Utils.h"

#define VERSION_STR "v 0.1.17 alpha"

int main(int argc, char* argv[]) {
	InitLog();

	Log("======== START: VERSION: %s AT: %s", VERSION_STR, TimeToString(time(NULL)).c_str());

	CmdLineParser* pCmdLineParser = new CmdLineParser(argc, argv);

	if (pCmdLineParser != NULL) {
		//check syntax and build command structure
		if (pCmdLineParser->Parse()) {
			Adapter* pAdapter = new RitexAdapter("Ritex", VERSION_STR, "Ritex adapter", pCmdLineParser);
			// go into the loop which either run the daemon or exits after command line parameter processing
			if(pAdapter) {
				pAdapter->Run();
				delete pAdapter;
			}
		}
	}
	Log("[main()] EXIT: AT: %s", TimeToString(time(NULL)).c_str());
	return 0;
}
