/*
 * DeviceCommandFactory.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DeviceCommandFactory.h"
#include "ShowInfoHostCommand.h"
#include "DaemonCommandChangeSetting.h"

#include <stdlib.h>
#include <vector>
#include <string>

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
			int cmd = atoi(cmdLineCommand.m_cmdType.c_str());
			switch (cmd) {
				case 3: //change setting
					//std::vector<std::string> x = std::string::split("one:two::three", ':');
					return new DaemonCommandChangeSetting(pAdapter,987,1.5);
				default:
					break;
			}
		}
		break;
	default:
		return NULL;
	}
}


