/*
 * Sensor.h
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include "DeviceChannel.h"
#include <list>

class Sensor {
protected:
	int m_id; //sensor ID
	std::list<DeviceChannel*> m_deviceChannelList;
public:
	Sensor();
	Sensor(int id);
	virtual ~Sensor();
	std::list<DeviceChannel*>& getChannels() { return m_deviceChannelList; };
	bool AddChannel(DeviceChannel* pChannel);
	int GetId() { return m_id; };
};

#endif /* SENSOR_H_ */
