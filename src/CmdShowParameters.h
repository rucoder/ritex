/*
 * HostCommandShowParameters.h
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#ifndef HOSTCOMMANDSHOWPARAMETERS_H_
#define HOSTCOMMANDSHOWPARAMETERS_H_

#include "DeviceCommand.h"
#include "Device.h"

class CmdShowParameters : public DeviceCommand {
public:
	CmdShowParameters(Device* device);
	virtual ~CmdShowParameters();
	virtual bool Execute();
private:
	Device* m_pDevice;
	void printParameter(AdapterParameter* pParam);
};

#endif /* HOSTCOMMANDSHOWPARAMETERS_H_ */
