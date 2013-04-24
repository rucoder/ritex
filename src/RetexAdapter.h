/*
 * RetexAdapter.h
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#ifndef RETEXADAPTER_H_
#define RETEXADAPTER_H_

#include "Adapter.h"

class RetexAdapter: public Adapter {
private:
	int OpenCommPort(std::string port, int speed);
public:
	RetexAdapter() {};
	RetexAdapter(CmdLineParser* parser);
	virtual ~RetexAdapter();
	bool isDaemonRunning();
	virtual int DaemonLoop();
	virtual int ParentLoop();
};

#endif /* RETEXADAPTER_H_ */
