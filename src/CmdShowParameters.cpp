/*
 * HostCommandShowParameters.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#include "CmdShowParameters.h"
#include <stdio.h>

CmdShowParameters::CmdShowParameters(Device* pDevice)
	: DeviceCommand(false), m_pDevice(pDevice)
{
}

CmdShowParameters::~CmdShowParameters() {
	// TODO Auto-generated destructor stub
}

bool CmdShowParameters::Execute()
{
	// loop through device channels
	Device* pDev = m_pDevice;

	for(std::list<DeviceChannel*>::const_iterator ch = pDev->getChannels().begin(); ch != pDev->getChannels().end(); ch++) {
		AdapterParameter* pParam = (*ch)->GetParameter();
		printf("%d-%d", (*ch)->GetId(), pParam->GetId());
		// following 2 lines are ugly
		std::list<DeviceChannel*>::const_iterator tmp = ch;
		tmp++;
		if(tmp != pDev->getChannels().end()) {
			printf(",");
		}
	}
	printf("\n");

	// loop through sensor's channels
	for(std::list<Sensor*>::const_iterator sn = pDev->getSensors().begin(); sn != pDev->getSensors().end(); sn++) {
		printf("%d|", (*sn)->GetId());
		for(std::list<DeviceChannel*>::iterator ch = (*sn)->getChannels().begin(); ch != (*sn)->getChannels().end(); ch++) {
			AdapterParameter* pParam = (*ch)->GetParameter();
			printf("%d-%d", (*ch)->GetId(), pParam->GetId());
			// following 2 lines are ugly
			std::list<DeviceChannel*>::iterator tmp = ch;
			tmp++;
			if(tmp != (*sn)->getChannels().end()) {
				printf(",");
			}
		}
		printf("\n");
	}

    return true;
}


