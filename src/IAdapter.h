/*
 * IAdapter.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef IADAPTER_H_
#define IADAPTER_H_

#include "EventLoggerThread.h"
#include "DataLoggerThread.h"
#include "CmdLoggerThread.h"

class IAdapter {
public:
	virtual const std::string& getName() = 0;
	virtual const std::string& getVersion() = 0;
	virtual const std::string& getDescription() = 0;
	virtual CmdLoggerThread* getCmdLogger() = 0;
	virtual EventLoggerThread* getEventLogger() = 0;
	virtual DataLoggerThread* getDataLogger() = 0;
//protected:
	virtual ~IAdapter() {};
};



#endif /* IADAPTER_H_ */
