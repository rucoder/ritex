/*
 * CmdTestDevice.h
 *
 *  Created on: May 3, 2013
 *      Author: mmalyshe
 */

#ifndef CMDTESTDEVICE_H_
#define CMDTESTDEVICE_H_

#include "DeviceCommand.h"
#include "RitexDevice.h"

class CmdTestDevice: public DeviceCommand {
protected:
	RitexDevice* m_pDevice;
	std::string m_commport;
	int m_speed;
public:
	CmdTestDevice(RitexDevice* p_device, std::string commport, int speed);
	virtual ~CmdTestDevice();
	virtual bool Execute();
	virtual void SetReply(DataPacket* packet, int status,DataPacket* param2 = NULL);
};

#endif /* CMDTESTDEVICE_H_ */
