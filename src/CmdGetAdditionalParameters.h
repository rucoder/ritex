/*
 * CmdGetSupportedParameters.h
 *
 *  Created on: Apr 27, 2013
 *      Author: ruinmmal
 */

#ifndef CMDGETSUPPORTEDPARAMETERS_H_
#define CMDGETSUPPORTEDPARAMETERS_H_

#include "IAdapter.h"
#include "DeviceCommand.h"

class CmdGetAdditionalParameters: public DeviceCommand {
protected:
	IAdapter* m_pAdapter;
public:
	CmdGetAdditionalParameters(IAdapter * pAdapter);
	virtual ~CmdGetAdditionalParameters();
	virtual bool Execute();
};

#endif /* CMDGETSUPPORTEDPARAMETERS_H_ */
