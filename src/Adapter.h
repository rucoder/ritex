/*
 * Adapter.h
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#ifndef ADAPTER_H_
#define ADAPTER_H_

#include <pthread.h>
#include "IAdapter.h"
#include "Device.h"
#include "CmdLineParser.h"
#include "AdapterParameter.h"
#include "DaemonCommChannel.h"
#include "EventLoggerThread.h"
#include "DataLoggerThread.h"
#include "CmdLoggerThread.h"

#define PID_FILE_PATH "/tmp/"

class Adapter: public IAdapter{
private:

	enum eExecutionContext {
		CONTEXT_DAEMON,
		CONTEXT_PARENT,
		CONTEXT_ERROR
	};

	eExecutionContext BecomeDaemon();
	int LockPidFile(const char* pidfile);
	void DeletePidFile();
	void GeneratePidFileName(int deviceId);


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
	DaemonCommChannel* m_pCommChannel;

	std::string m_socket;
	std::string m_pidFileName;
	std::string m_pidFilePath;

	// logging facilities
	EventLoggerThread* m_pEventLogger;
	DataLoggerThread* m_pDataLogger;
	CmdLoggerThread* m_pCmdLogger;
	bool CreateLoggerFacility();
public:
	Adapter(std::string name, std::string version, std::string description, CmdLineParser* parser);
	bool AddParameter(AdapterParameter* parameter);
	virtual ~Adapter();
	virtual int Run();
	//may be useful to create interactive application to test daemon or for unit tests
	virtual int ParentLoop(bool isCommOk);
	bool UpdateParameterFilter(int deviId);
	DaemonCommChannel* GetCommChannel() { return m_pCommChannel; };

	//From IAdapter
	const std::string& getName() { return m_adapterName; };
	const std::string& getVersion() { return m_adapterVersion; };
	const std::string& getDescription() { return m_adapterDescription; };
	virtual CmdLoggerThread* getCmdLogger() { return m_pCmdLogger; };
	virtual EventLoggerThread* getEventLogger() { return m_pEventLogger; };
	virtual DataLoggerThread* getDataLogger() { return m_pDataLogger; };

};

#endif /* ADAPTER_H_ */
