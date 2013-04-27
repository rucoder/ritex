/*
 * Device.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#include "Device.h"
#include <stdio.h>
#include <assert.h>


#include "ShowInfoHostCommand.h"
#include "HostCommandShowParameters.h"
#include "CmdGetAdditionalParameters.h"

Device::Device(IAdapter* pAdapter) :
	m_deviceId(-1), // make it invalid
	m_pAdapter(pAdapter)
{
}

Device::~Device() {
	// TODO Auto-generated destructor stub
}

bool Device::AddSensor(Sensor* pSensor)
{
	m_sensorList.push_back(pSensor);
	return true;
}

bool Device::AddChannel(DeviceChannel* pChannel)
{
	m_deviceChannelList.push_back(pChannel);
	return true;
}


DeviceCommand* Device::CreateCommand(CmdLineCommand* cmd)
{
	//handle common commands like -p, -cmd, -d
	switch(cmd->m_cmdLineCommandType)
	{
	case CMD_SHOW_INFO:
		return new ShowInfoHostCommand(this, m_pAdapter);
	case CMD_GET_CONNECTED_DEVICE_INFO:
		return new HostCommandShowParameters(this);
	case CMD_GET_ADDITIONAL_PARAMETER_LIST:
		return new CmdGetAdditionalParameters(m_pAdapter);
	default:
		printf("WARNING: NOT SUPPORTED: cmdLineCommand.m_cmdLineCommandType == %d\n", cmd->m_cmdLineCommandType);
		return NULL;
	}
}





