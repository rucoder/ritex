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

EventLoggerThread::EventLoggerThread(std::string db_name)
	: LoggerThread<DBEventCommon*>(db_name)
{
	m_sSqlQuery = "insert  into tbleventbus (TypeId, ChannelId, RegisterDate, Argument1, Argument2, Argument3, Argument4) values(?,?,?,?,?,?,?)";
}

bool EventLoggerThread::Insert(DBEventCommon* event)
{
	//bind parameters first
	sqlite3_bind_int(m_pStm, 1, event->getTypeId());
	sqlite3_bind_int(m_pStm, 2, event->getChannelId());
	sqlite3_bind_text(m_pStm, 3, Utils::TimeToString(event->getRegisterTimeDate()).c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pStm, 4, event->getArgument1().c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pStm, 5, event->getArgument2().c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pStm, 6, event->getArgument3().c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pStm, 7, event->getArgument4().c_str(), -1, SQLITE_STATIC);

	//now execute query
	return true;
}
