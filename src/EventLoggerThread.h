/*
 * EventLoggerThread.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef EVENTLOGGERTHREAD_H_
#define EVENTLOGGERTHREAD_H_

#include "LoggerThread.hpp"
#include "DBEventCommon.h"

class EventLoggerThread: public LoggerThread<DBEventCommon*> {
protected:
	EventLoggerThread() {};
	virtual bool BindParams(DBEventCommon* data);
public:
	EventLoggerThread(std::string db_name);
	virtual ~EventLoggerThread() {};
};

#endif /* EVENTLOGGERTHREAD_H_ */
