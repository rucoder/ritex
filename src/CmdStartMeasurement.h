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
	struct __sm_serial: public __serial_data {

	};
	virtual void Serialize();
	CmdStartMeasurement();
public:
	CmdStartMeasurement(RitexDevice* p_device);
	virtual ~CmdStartMeasurement();
	virtual bool Execute();
private:
	RitexDevice* m_pDevice;
};

#endif /* CMDSTARTMEASUREMENT_H_ */
