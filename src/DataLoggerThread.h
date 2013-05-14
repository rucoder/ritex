/*
 * DataLoggerThread.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef DATALOGGERTHREAD_H_
#define DATALOGGERTHREAD_H_

#include "LoggerThread.hpp"
#include "DBDataPacket.h"

class DataLoggerThread: public LoggerThread<DBDataPacket*> {
protected:
	DataLoggerThread() {};
	virtual bool BindParams(DBDataPacket* event);
public:
	DataLoggerThread(std::string db_name);
	virtual ~DataLoggerThread() {};
};

#endif /* DATALOGGERTHREAD_H_ */
