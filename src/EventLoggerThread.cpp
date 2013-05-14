/*
 * EventLoggerThread.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "EventLoggerThread.h"
#include "Utils.h"
#include <syslog.h>

#include <assert.h>
#include "sqlite3.h"

EventLoggerThread::EventLoggerThread(std::string db_name)
	: LoggerThread<DBEventCommon*>(db_name, "EventLoggerThread")
{
	m_sSqlQuery = "insert  into tbleventbus (TypeId, ChannelId, RegisterDate, Argument1, Argument2, Argument3, Argument4) values(?,?,?,?,?,?,?)";
}

bool EventLoggerThread::BindParams(DBEventCommon* event)
{
	return Bind(1, event->getTypeId()) && Bind(2, event->getChannelId()) &&
	Bind(3, TimeToString(event->getRegisterTimeDate())) &&
	Bind(4, event->getArgument1())&&
	Bind(5, event->getArgument2())&&
	Bind(6, event->getArgument3())&&
	Bind(7, event->getArgument4());
}
