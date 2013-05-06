/*
 * CmdStartMeasurement.h
 *
 *  Created on: Apr 26, 2013
 *      Author: ruinmmal
 */

#ifndef CMDSTARTMEASUREMENT_H_
#define CMDSTARTMEASUREMENT_H_

#include "DeviceCommand.h"
#include "RitexDevice.h"

class CmdStartMeasurement: public DeviceCommand {
protected:
	CmdStartMeasurement();
	std::string m_commport;
	int m_speed;
public:
	CmdStartMeasurement(RitexDevice* p_device, std::string commport, int speed);
	virtual ~CmdStartMeasurement();
	virtual bool Execute();
	virtual void SetReply(DataPacket* packet, int status) {
		NotifyResultReady();
	}

private:
	RitexDevice* m_pDevice;
};

#endif /* CMDSTARTMEASUREMENT_H_ */
