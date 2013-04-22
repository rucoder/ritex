/*
 * HostCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "HostCommand.h"

HostCommand::HostCommand() {
	// TODO Auto-generated constructor stub

}

HostCommand::HostCommand(Adapter* adapter) : DeviceCommand(adapter, false)
{

}


HostCommand::~HostCommand() {
	// TODO Auto-generated destructor stub
}

