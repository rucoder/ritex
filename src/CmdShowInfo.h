/*
 * ShowInfoHostCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef SHOWINFOHOSTCOMMAND_H_
#define SHOWINFOHOSTCOMMAND_H_

#include "DeviceCommand.h"
#include "Device.h"
#include "AdapterParameter.h"
#include "IAdapter.h"

class CmdShowInfo: public DeviceCommand {
public:
	CmdShowInfo(Device* device, IAdapter* adapter);
	virtual ~CmdShowInfo();
	virtual bool Execute();
private:
	void printParameter(AdapterParameter* pParam, bool isDeviceChannel);
	Device* m_pDevice;
	IAdapter* m_pAdapter;
};

#endif /* SHOWINFOHOSTCOMMAND_H_ */
