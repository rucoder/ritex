/*
 * DeviceCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include <stdlib.h>
#include "DeviceCommand.h"

#include <assert.h>

DeviceCommand::DeviceCommand(bool isHwCommand)
	: m_isHWCommand(isHwCommand), m_rawResult(NULL), m_rawResultLength(0), m_pListener(NULL)
{

}


DeviceCommand::~DeviceCommand() {
	// TODO Auto-generated destructor stub
}

void DeviceCommand::NotifyResultReady()
{
	if(m_pListener) {
		m_pListener->OnResultReady(this);
	}
}






