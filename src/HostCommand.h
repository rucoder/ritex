/*
 * HostCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef HOSTCOMMAND_H_
#define HOSTCOMMAND_H_

#include "DeviceCommand.h"

class HostCommand: public DeviceCommand {
protected:
	HostCommand();
public:
	HostCommand(Adapter* adapter);
	virtual ~HostCommand();
	//virtual bool Execute();
};

#endif /* HOSTCOMMAND_H_ */
