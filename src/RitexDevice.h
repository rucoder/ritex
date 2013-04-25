/*
 * RitexDevice.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef RITEXDEVICE_H_
#define RITEXDEVICE_H_

#include "Device.h"
#include "ComTrafficProcessor.h"

class RitexDevice: public Device {
protected:
	RitexDevice();
	ComTrafficProcessor* m_pProcessor;
public:
	RitexDevice(IAdapter* pAdapter);
	virtual ~RitexDevice();
	DeviceCommand* CreateCommand(void* rawCommand, int length);
	DeviceCommand* CreateCommand(CmdLineCommand* cmd);
	bool StartMesurements();
};

#endif /* RITEXDEVICE_H_ */
