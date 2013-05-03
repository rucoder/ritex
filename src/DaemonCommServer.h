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
#include "CmdResulReadytListener.h"
#include "DeviceStatusListener.h"

class DaemonCommServer : public Thread , public ICmdResulReadytListener, public IDeviceStatusListener {
private:
	Device* m_pDevice;
	IAdapter* m_pAdapter;
	int m_connectSock;
	int m_clientSock;
	pthread_cond_t m_canProcessCommandsCond;
	pthread_mutex_t m_mutex;
	bool m_canProcessCommands;
protected:
	virtual void* Run();
	virtual void OnCancel();
public:
	DaemonCommServer(Device* pDevice, IAdapter* pAdapter);
	virtual ~DaemonCommServer();
	virtual bool Create(bool createDetached = false);
	virtual void OnResultReady(DeviceCommand* pCmd);
	virtual void OnDeviceStatusChanged(eDeviceStaus status);
};

#endif /* DAEMONCOMMSERVER_H_ */
