/*
 * Adapter.h
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#ifndef ADAPTER_H_
#define ADAPTER_H_

#include <pthread.h>
#include "CmdLineParser.h"
#include "AdapterParameter.h"
#include "Device.h"
#include "DaemonCommChannel.h"
#include <list>
class Adapter {
private:

	enum eExecutionContext {
		CONTEXT_DAEMON,
		CONTEXT_PARENT,
		CONTEXT_ERROR
	};
	eExecutionContext BecomeDaemon();

	int LockPidFile(const char* pidfile);


protected:
	Adapter() {};

	std::string m_adapterName;
	std::string m_adapterVersion;
	std::string m_adapterDescription;

	CmdLineParser * m_pCmdLineParser;

	// device attached
	Device* m_pDevice;

	// supported additional parameters. assume all options accept all parameters
	//TODO:

	//supported commands

	// for daemon communication
	// TODO: hide somehow. make abstract channel
	std::string m_socket;
	DaemonCommChannel* m_pCommChannel;

public:
	Adapter(CmdLineParser* parser);
	bool AddParameter(AdapterParameter* parameter);
	virtual ~Adapter();
	virtual int Run();
	virtual int DaemonLoop() = 0;
	virtual int ParentLoop() = 0;
	bool UpdateChannelList(int deviId);
	const std::string& GetName() { return m_adapterName; };
	const std::string& GetVersion() { return m_adapterVersion; };
	const std::string& GetDescription() { return m_adapterDescription; };
	Device* GetDevice() { return m_pDevice; }
	DaemonCommChannel* GetCommChannel() { return m_pCommChannel; };
};

#endif /* ADAPTER_H_ */
