/*
 * RetexAdapter.h
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#ifndef RETEXADAPTER_H_
#define RETEXADAPTER_H_

#include "Adapter.h"

class RitexAdapter: public Adapter {
private:
	int OpenCommPort(std::string port, int speed);
	static const char* m_commDevices[];
	static const char* m_commSpeed[];
public:
	RitexAdapter() {};
	RitexAdapter(std::string name, std::string version, std::string description, CmdLineParser* parser);
	virtual ~RitexAdapter();
	bool isDaemonRunning();
	int DaemonLoop();
	virtual int ParentLoop(bool isCommOk);
};

#endif /* RETEXADAPTER_H_ */
