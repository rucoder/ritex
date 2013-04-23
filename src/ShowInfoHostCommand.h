/*
 * ShowInfoHostCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef SHOWINFOHOSTCOMMAND_H_
#define SHOWINFOHOSTCOMMAND_H_

#include "HostCommand.h"

class ShowInfoHostCommand: public HostCommand {
public:
	ShowInfoHostCommand();
	ShowInfoHostCommand(Adapter* adapter);
	virtual ~ShowInfoHostCommand();
	virtual bool Execute();
private:
	void printParameter(AdapterParameter* pParam, bool isDeviceChannel);
};

#endif /* SHOWINFOHOSTCOMMAND_H_ */
