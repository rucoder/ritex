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

class DeviceChannel {
protected:
	int m_channelId;
	int m_paramId;
	int m_sensorId;
	int m_sensorChannelId;
	bool m_isDeviceChannel;
	bool m_isEventDriven;
	std::string m_interfaceArg1;
	std::string m_interfaceArg2;
	std::string m_interfaceArg3;
	std::string m_interfaceArg4;

	std::map<std::string, float> m_ValueRangeMap;
public:
	DeviceChannel();
	virtual ~DeviceChannel();

	DeviceChannel(int chId, int paramId, int sensorId, int sensorChannel, bool isEventDriven
			, std::string iarg1, std::string iarg2,std::string iarg3,std::string iarg4);
	bool SetValueRange(std::string name, float value);

	bool GetValueRange(std::string name, float& value);

	static DeviceChannel* CreateChannelFromString(std::string str);

	std::string GetSerialKey(int devId);
};

#endif /* DEVICECHANNEL_H_ */
