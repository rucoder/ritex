/*
 * Sensor.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#include "Sensor.h"

Sensor::Sensor() {
	// TODO Auto-generated constructor stub

}

Sensor::Sensor(int id)
	: m_id(id)
{
}

Sensor::~Sensor() {
	// TODO Auto-generated destructor stub
}

bool Sensor::AddChannel(DeviceChannel* pChannel)
{
	m_deviceChannelList.push_back(pChannel);
}


