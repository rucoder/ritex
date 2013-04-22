/*
 * DeviceChannel.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DeviceChannel.h"
#include <stdlib.h>

DeviceChannel::DeviceChannel() {
	// TODO Auto-generated constructor stub

}

DeviceChannel::~DeviceChannel() {
	// TODO Auto-generated destructor stub
}

DeviceChannel::DeviceChannel(int chId, int paramId, int sensorId, int sensorChannel, bool isEventDriven
		, std::string iarg1, std::string iarg2,std::string iarg3,std::string iarg4) :
				m_channelId(chId)
{

}
bool DeviceChannel::SetValueRange(std::string name, float value)
{
	m_ValueRangeMap[name] = value;
	return true;
}

bool DeviceChannel::GetValueRange(std::string name, float& value)
{
	std::map<std::string, float>::iterator it = m_ValueRangeMap.find(name);

	if (it == m_ValueRangeMap.end())
		return false;

	value = it->second;

	return true;
}

DeviceChannel*  DeviceChannel::CreateChannelFromString(std::string str)
{
	//TODO: parse script response
	return NULL;
}

std::string DeviceChannel::GetSerialKey(int devId)
{
	return "";
}


