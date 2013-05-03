/*
 * ShowInfoHostCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "ShowInfoHostCommand.h"
#include <stdio.h>


ShowInfoHostCommand::ShowInfoHostCommand(Device* device, IAdapter* adapter)
	: DeviceCommand(false), m_pDevice(device), m_pAdapter(adapter)
{

}

ShowInfoHostCommand::~ShowInfoHostCommand() {
	// TODO Auto-generated destructor stub
}

void ShowInfoHostCommand::printParameter(AdapterParameter* pParam, bool isDeviceChannel)
{
	printf("%d|%s|%d|%d|%s|%d\n", pParam->GetId(), pParam->GetName().c_str(), isDeviceChannel ? 1 : 0,
			pParam->isOverridable() ? 1 : 0, pParam->GetArgs().c_str(),  pParam->isEventDriven() ? 1:0);
}


bool ShowInfoHostCommand::Execute()
{

	printf("\"%s\"|\"%s\"|\"%s\"\n", m_pAdapter->getName().c_str(), m_pAdapter->getVersion().c_str(),m_pAdapter->getDescription().c_str());

	// loop through device channels
	Device* pDev = m_pDevice;

	for(std::list<DeviceChannel*>::const_iterator ch = pDev->getChannels().begin(); ch != pDev->getChannels().end(); ch++) {
		AdapterParameter* pParam = (*ch)->GetParameter();
		printParameter(pParam, true);
	}

	// loop through sensor's channels
	for(std::list<Sensor*>::const_iterator sn = pDev->getSensors().begin(); sn != pDev->getSensors().end(); sn++) {
		for(std::list<DeviceChannel*>::iterator ch = (*sn)->getChannels().begin(); ch != (*sn)->getChannels().end(); ch++) {
			printParameter((*ch)->GetParameter(),false);
		}
	}

    return true;
}


