/*
 * DeviceCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DEVICECOMMAND_H_
#define DEVICECOMMAND_H_

#include "DataPacket.h"
#include "CmdResulReadytListener.h"
#include <list>
#include "CmdLineCommand.h"

class DeviceCommand {
private:
	bool m_isHWCommand;
public:

protected:
	std::string m_rawResult;
	std::list<ICmdResulReadytListener*> m_Listeners;
	CmdLineCommand* m_pParentCommand;
	virtual void NotifyResultReady();
	time_t m_arrivalTime;
	time_t m_finishedTime;
public:
	DeviceCommand(bool isHw, CmdLineCommand* parent = NULL);
	virtual ~DeviceCommand();
	bool isHWCommand() { return m_isHWCommand; };
	const std::string& getRawResult() const { return m_rawResult; }
	virtual bool Execute() = 0;
	void AddResultListener(ICmdResulReadytListener* pListener) {
		m_Listeners.push_back(pListener);
	}
	virtual void SetReply(DataPacket* packet, int status, DataPacket* param2 = NULL);
	CmdLineCommand* GetParentCmd() { return m_pParentCommand; }
	time_t GetArrivalTime() { return m_arrivalTime; }
	time_t GetFinishedTime() { return m_finishedTime; }
};

#endif /* DEVICECOMMAND_H_ */
