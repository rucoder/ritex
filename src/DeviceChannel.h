/*
 * DeviceChannel.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DEVICECHANNEL_H_
#define DEVICECHANNEL_H_

#include <map>
#include <string>

#include "AdapterParameter.h"

class DeviceChannel {
protected:
	int m_channelId;
	AdapterParameter* m_param;
	//int m_sensorId;
	//int m_sensorChannelId;
	//bool m_isDeviceChannel;
	bool m_isEventDriven;
	std::string m_interfaceArg1;
	std::string m_interfaceArg2;
	std::string m_interfaceArg3;
	std::string m_interfaceArg4;

	std::map<std::string, float> m_ValueRangeMap;
public:
	virtual ~DeviceChannel();

	DeviceChannel(int chId, bool isEventDriven, AdapterParameter* param);
	AdapterParameter* GetParameter() { return m_param; };
	int GetId() {return m_channelId; };
	//bool SetValueRange(std::string name, float value);

	///bool GetValueRange(std::string name, float& value);

	//static DeviceChannel* CreateChannelFromString(std::string str);

	std::string GetSerialKey(int devId);
};

#endif /* DEVICECHANNEL_H_ */
