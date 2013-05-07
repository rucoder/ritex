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
#include "ParameterFilter.h"
#include "AdapterCommand.h"

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
	bool CreateLoggerFacility();

	// supported parameters
	std::map<std::string, struct additional_parameter_t*> m_additionalParameters;

	ParameterFilter m_paramFilter;

	virtual void DaemonCleanup();


public:
	Adapter(std::string name, std::string version, std::string description, CmdLineParser* parser);
	//bool AddParameter(AdapterParameter* parameter);
	bool AddAdditionalParameter(std::string name, int a, int b);
	bool AddAdditionalParameterFloat(std::string name, float a, float b);
	bool AddAdditionalParameter(std::string name, std::string value);
	bool AddAdditionalParameter(std::string name, const char* const list[], int size, eListValueType type);



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
	virtual EventLoggerThread* getEventLogger() { return m_pEventLogger; };
	virtual DataLoggerThread* getDataLogger() { return m_pDataLogger; };
	virtual std::map<std::string, struct additional_parameter_t*>& GetAdditionalParameterMap() {return m_additionalParameters;}
	virtual const ParameterFilter& GetParameterFilter() const {
		return m_paramFilter;
	}


};

#endif /* ADAPTER_H_ */
