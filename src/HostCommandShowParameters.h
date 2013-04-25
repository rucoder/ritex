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

class HostCommandShowParameters : public DeviceCommand {
protected:
	HostCommandShowParameters();
public:
	HostCommandShowParameters(Device* device);
	virtual ~HostCommandShowParameters();
	virtual bool Execute();
private:
	Device* m_pDevice;
	void printParameter(AdapterParameter* pParam);
};

#endif /* HOSTCOMMANDSHOWPARAMETERS_H_ */
