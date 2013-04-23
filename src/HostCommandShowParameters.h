/*
 * HostCommandShowParameters.h
 *
 *  Created on: Apr 23, 2013
 *      Author: ruinmmal
 */

#ifndef HOSTCOMMANDSHOWPARAMETERS_H_
#define HOSTCOMMANDSHOWPARAMETERS_H_

#include "HostCommand.h"

class HostCommandShowParameters : public HostCommand {
public:
	HostCommandShowParameters();
	HostCommandShowParameters(Adapter* pAdapter);
	virtual ~HostCommandShowParameters();
	virtual bool Execute();
private:
	void printParameter(AdapterParameter* pParam);
};

#endif /* HOSTCOMMANDSHOWPARAMETERS_H_ */
