/*
 * CmdResulReadytListener.h
 *
 *  Created on: May 4, 2013
 *      Author: ruinmmal
 */

#ifndef CMDRESULREADYTLISTENER_H_
#define CMDRESULREADYTLISTENER_H_

#include "DeviceCommand.h"

class DeviceCommand;

class ICmdResulReadytListener {
protected:
	virtual ~ICmdResulReadytListener() {}
public:
	virtual void OnResultReady(DeviceCommand* pCmd) = 0;
};

#endif /* CMDRESULREADYTLISTENER_H_ */
