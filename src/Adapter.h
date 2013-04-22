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
#include "DeviceChannel.h"
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
	std::list<AdapterParameter*> m_parameterList;

	//TODO: should we support several list for several device IDs?
	std::list<DeviceChannel*> m_channelList;

	void printAdapterInfo();
	void printSupportedCommands();
	void printSupportedParameters();

	// for daemon communication
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
	const std::list<AdapterParameter*> GetParameterList() { return m_parameterList; }
	DaemonCommChannel* GetCommChannel() { return m_pCommChannel; };
};

#endif /* ADAPTER_H_ */
