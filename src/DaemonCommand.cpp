/*
 * DaemonCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DaemonCommand.h"

DaemonCommand::DaemonCommand() : DeviceCommand(true)
{
	// TODO Auto-generated constructor stub

}

DaemonCommand::DaemonCommand(Adapter* adapter) : DeviceCommand(adapter, true)
{

}

DaemonCommand::~DaemonCommand() {
	// TODO Auto-generated destructor stub
}

bool DaemonCommand::Execute()
{
	return true;
}


