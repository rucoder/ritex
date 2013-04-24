/*
 * DeviceCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DeviceCommand.h"

DeviceCommand::DeviceCommand() {
	// TODO Auto-generated constructor stub

}

DeviceCommand::DeviceCommand(Adapter* adapter, bool needDaemon)
	: m_pAdapter(adapter), m_isNeedDaemon(needDaemon)
{

}

DeviceCommand::DeviceCommand(bool needDaemon)
: m_pAdapter(NULL), m_isNeedDaemon(needDaemon)
{

}


DeviceCommand::~DeviceCommand() {
	// TODO Auto-generated destructor stub
}

