/*
 * DaemonCommandChangeSetting.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DAEMONCOMMANDCHANGESETTING_H_
#define DAEMONCOMMANDCHANGESETTING_H_

#include "DaemonCommand.h"

class DaemonCommandChangeSetting: public DaemonCommand {
protected:
	int m_setting;
	float m_value;
public:
	DaemonCommandChangeSetting();
	DaemonCommandChangeSetting(Adapter* adapter, int setting, float value);
	virtual ~DaemonCommandChangeSetting();
	virtual bool Execute();
};

#endif /* DAEMONCOMMANDCHANGESETTING_H_ */
