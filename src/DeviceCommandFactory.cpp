/*
 * DeviceCommandFactory.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DeviceCommandFactory.h"
#include "ShowInfoHostCommand.h"
#include "DaemonCommandChangeSetting.h"
#include "HostCommandShowParameters.h"

#include <stdlib.h>
#include <vector>
#include <string>

#include <stdio.h>
DeviceCommandFactory::DeviceCommandFactory() {
	// TODO Auto-generated constructor stub

}

DeviceCommandFactory::~DeviceCommandFactory() {
	// TODO Auto-generated destructor stub
}

DeviceCommand* DeviceCommandFactory::CreateDeviceCommand(CmdLineCommand& cmdLineCommand, Adapter* pAdapter)
{
	switch(cmdLineCommand.m_cmdLineCommandType)
	{
	case CMD_SHOW_INFO:
		return new ShowInfoHostCommand(pAdapter);
	case CMD_COMMAND:
		{
			//TODO: assign functions for command creation to supported command list or at least make index definitions
			switch (cmdLineCommand.m_cmdId) {
				case 3: //change setting
					// TODO: get values from command
					return new DaemonCommandChangeSetting(pAdapter,987,1.5);
				default:
					break;
			}
		}
		break;
	case CMD_GET_CONNECTED_DEVICE_INFO:
		return new HostCommandShowParameters(pAdapter);
	case CMD_GET_MESUREMENTS:
		// dummy implementation since we just need start daemon
		return new DaemonCommand();
	default:
		printf("WARNING: NOT SUPPORTED: cmdLineCommand.m_cmdLineCommandType == %d\n", cmdLineCommand.m_cmdLineCommandType);
		return NULL;
	}
}


