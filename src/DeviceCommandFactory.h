/*
 * DeviceCommandFactory.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DEVICECOMMANDFACTORY_H_
#define DEVICECOMMANDFACTORY_H_

#include "CmdLineCommand.h"
#include "DeviceCommand.h"

class DeviceCommandFactory {
private:
	DeviceCommandFactory();
	virtual ~DeviceCommandFactory();
public:
	static DeviceCommand* CreateDeviceCommand(CmdLineCommand& cmdLineCommand, Adapter* pAdapter);
};

#endif /* DEVICECOMMANDFACTORY_H_ */
