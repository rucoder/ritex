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

class DeviceCommand {
private:
	bool m_isHWCommand;
public:

protected:
	char* m_rawResult;
	int m_rawResultLength;
	ICmdResulReadytListener* m_pListener;
	virtual void NotifyResultReady();
public:
	DeviceCommand(bool isHw);
	virtual ~DeviceCommand();
	bool isHWCommand() { return m_isHWCommand; };
	char* getRawResult() { return m_rawResult; };
	int getRawResultLength() { return m_rawResultLength; };
	virtual bool Execute() = 0;
	void SetResultListener(ICmdResulReadytListener* pListener) {
		m_pListener = pListener;
	}
	virtual void SetReply(DataPacket* packet, int status) { NotifyResultReady(); };
};

#endif /* DEVICECOMMAND_H_ */
