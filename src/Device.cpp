/*
 * Device.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#include "Device.h"

Device::Device() :
	m_deviceId(-1) // make it invalid
{
	// TODO Auto-generated constructor stub

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


