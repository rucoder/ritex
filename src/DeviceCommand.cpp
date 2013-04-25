/*
 * DeviceCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include <stdlib.h>
#include "DeviceCommand.h"

#include <assert.h>

DeviceCommand::DeviceCommand()
	: m_isHWCommand(false), m_rawResult(NULL), m_rawResultLength(0),
	  m_rawCommand(NULL), m_rawCommandLength(0), m_cmdId(-1)
{
	// TODO Auto-generated constructor stub

}

DeviceCommand::DeviceCommand(bool isHwCommand)
	: m_isHWCommand(isHwCommand), m_rawResult(NULL), m_rawResultLength(0),
	  m_rawCommand(NULL), m_rawCommandLength(0), m_cmdId(-1)
{

}


DeviceCommand::~DeviceCommand() {
	// TODO Auto-generated destructor stub
}

unsigned char* DeviceCommand::getRawCommand()
{
	assert(m_isHWCommand);

	if(m_rawCommandLength == 0) {
		Serialize();
	}
	return m_rawCommand;
};

int DeviceCommand::getRawCommandLength()
{
	assert(m_isHWCommand);

	if(m_rawCommandLength == 0) {
		Serialize();
	}
	return m_rawCommandLength;
}



