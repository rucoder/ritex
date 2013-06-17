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
#include "backtrace.h"

#ifndef _ADAPTER_VERSION_
#define VERSION_STR "v0.1.17 alpha"
#else
#define VERSION_STR _ADAPTER_VERSION_
#endif

int main(int argc, char* argv[]) {
	install_sigsegv();
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
