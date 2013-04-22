/*
 * RetexAdapter.h
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#ifndef RETEXADAPTER_H_
#define RETEXADAPTER_H_

#include "Adapter.h"

#define PID_FILE "/home/ruinmmal/ritex.pid"

class RetexAdapter: public Adapter {
public:
	RetexAdapter() {};
	RetexAdapter(CmdLineParser* parser);
	virtual ~RetexAdapter();
	bool isDaemonRunning();
	virtual int DaemonLoop();
	virtual int ParentLoop();
};

#endif /* RETEXADAPTER_H_ */
