/*
 * DaemonCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DAEMONCOMMAND_H_
#define DAEMONCOMMAND_H_

#include "DeviceCommand.h"

class DaemonCommand: public DeviceCommand {
public:
	DaemonCommand();
	DaemonCommand(Adapter* adapter);
	virtual ~DaemonCommand();
};

#endif /* DAEMONCOMMAND_H_ */
