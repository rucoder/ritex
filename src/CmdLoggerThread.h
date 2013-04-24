/*
 * CmdLoggerThread.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef CMDLOGGERTHREAD_H_
#define CMDLOGGERTHREAD_H_

#include "LoggerThread.hpp"
#include "DBEventCommand.h"

class CmdLoggerThread: public LoggerThread<DBEventCommand*> {
protected:
	CmdLoggerThread() {};
	virtual bool Insert(DBEventCommand* event);
public:
	CmdLoggerThread(std::string db_name);
	virtual ~CmdLoggerThread() {};
};

#endif /* CMDLOGGERTHREAD_H_ */
