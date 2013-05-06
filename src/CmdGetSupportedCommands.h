/*
 * CmdGetSupportedCommands.h
 *
 *  Created on: May 5, 2013
 *      Author: ruinmmal
 */

#ifndef CMDGETSUPPORTEDCOMMANDS_H_
#define CMDGETSUPPORTEDCOMMANDS_H_

#include "Device.h"
#include "DeviceCommand.h"

class CmdGetSupportedCommands: public DeviceCommand {
public:
	CmdGetSupportedCommands(Device* device);
	virtual ~CmdGetSupportedCommands();
	virtual bool Execute();
private:
	Device* m_pDevice;
};

#endif /* CMDGETSUPPORTEDCOMMANDS_H_ */
