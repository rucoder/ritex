/*
 * CmdExternal.h
 *
 *  Created on: May 6, 2013
 *      Author: ruinmmal
 */

#ifndef CMDEXTERNAL_H_
#define CMDEXTERNAL_H_

#include "DeviceCommand.h"
#include "CmdLineCommand.h"
#include "RitexDevice.h"

#include <string>

class CmdExternal: public DeviceCommand {
protected:
	RitexDevice* m_pDevice;
	unsigned short m_cmdId;
	unsigned char m_param1;
	unsigned short m_param2;
	std::string m_commport;
	int m_speed;
public:

	CmdExternal(RitexDevice* device, std::string commport, int speed, CmdLineCommand* cmd, unsigned short cmdId, unsigned char param1, unsigned short param2 = 0);
	virtual ~CmdExternal();
	virtual bool Execute();
	virtual void SetReply(DataPacket* packet, int status);
};

#endif /* CMDEXTERNAL_H_ */
