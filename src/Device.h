/*
 * Device.h
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include "DeviceChannel.h"
#include "Sensor.h"

#include <list>

//consists of channels and sensors. each sensor has its own channels
class Device {
protected:
	std::list<Sensor*> m_sensorList;
	std::list<DeviceChannel*> m_deviceChannelList;
	int m_deviceId; // ID generated by installation system. Not set until passed from command line
public:
	Device();
	virtual ~Device();
	bool AddSensor(Sensor* pSensor);
	bool AddChannel(DeviceChannel* pChannel);
	std::list<Sensor*>& getSensors() { return m_sensorList; };
	std::list<DeviceChannel*>& getChannels() { return m_deviceChannelList; };
	void SetDeviceId(int devId) { m_deviceId = devId; };
};

#endif /* DEVICE_H_ */
