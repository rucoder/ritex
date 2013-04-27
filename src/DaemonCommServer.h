/*
 * DaemonCommServer.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef DAEMONCOMMSERVER_H_
#define DAEMONCOMMSERVER_H_

#include "Thread.h"
#include "Device.h"
#include "IAdapter.h"

class DaemonCommServer : public Thread {
private:
	Device* m_pDevice;
	IAdapter* m_pAdapter;
	int m_connectSock;
	int m_clientSock;
protected:
	virtual void* Run();
public:
	DaemonCommServer(Device* pDevice, IAdapter* pAdapter);
	virtual ~DaemonCommServer();
	virtual bool Create(bool createDetached = false);
};

#endif /* DAEMONCOMMSERVER_H_ */
