/*
 * DeviceChannel.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DeviceChannel.h"
#include <stdlib.h>

DeviceChannel::~DeviceChannel() {
	// TODO Auto-generated destructor stub
}

DeviceChannel::DeviceChannel(int chId, bool isEventDriven, AdapterParameter* param) :
				m_channelId(chId), m_param(param), m_isEventDriven(isEventDriven)
{

}



