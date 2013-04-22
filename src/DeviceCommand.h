/*
 * DeviceCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DEVICECOMMAND_H_
#define DEVICECOMMAND_H_

#include "Adapter.h"

class DeviceCommand {
protected:
	Adapter* m_pAdapter;
	DeviceCommand();
	bool m_isNeedDaemon;
public:
	DeviceCommand(Adapter* adapter, bool needDaemon);
	virtual ~DeviceCommand();
	virtual bool Execute() = 0;
	bool isNeedDaemon() { return m_isNeedDaemon; };
};

#endif /* DEVICECOMMAND_H_ */
