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
};

#endif /* DEVICECHANNEL_H_ */
