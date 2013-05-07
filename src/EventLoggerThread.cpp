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
	: LoggerThread<DBEventCommon*>(db_name)
{
	m_sSqlQuery = "insert  into tbleventbus (TypeId, ChannelId, RegisterDate, Argument1, Argument2, Argument3, Argument4) values(?,?,?,?,?,?,?)";
}


//bool CmdLoggerThread::Insert(DBEventCommand* event)
//{
//	syslog(LOG_ERR, "[SQL] Inserting CMD EVENT data");
//
//	//bind parameters first
//	if(sqlite3_bind_int(m_pStm, 1, event->m_cmdId) != SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind int 1");
//		return false;
//	}
//
//	if(sqlite3_bind_text(m_pStm, 2, event->m_src.c_str(), -1, SQLITE_TRANSIENT)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind text 1");
//		return false;
//	}
//
//	if(sqlite3_bind_int(m_pStm, 3, event->m_state)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind int 2");
//		return false;
//	}
//
//	if(sqlite3_bind_int(m_pStm, 4, event->m_type)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind int 3");
//		return false;
//	}
//
//	if(sqlite3_bind_text(m_pStm, 5, event->m_msg.c_str(), -1, SQLITE_TRANSIENT)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind text 2");
//		return false;
//	}
//
//
//	if(sqlite3_bind_int(m_pStm, 6, event->m_prio)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind int 4");
//		return false;
//	}
//
//	if(sqlite3_bind_int(m_pStm, 7, event->m_devId)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind int 5");
//		return false;
//	}
//
//
//	if(sqlite3_bind_text(m_pStm, 8, Utils::TimeToString(event->m_arrivalTime).c_str(), -1, SQLITE_TRANSIENT)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind time 1");
//		return false;
//	}
//
//	if(sqlite3_bind_text(m_pStm, 9, Utils::TimeToString(event->m_finishedTime).c_str(), -1, SQLITE_TRANSIENT)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind time 2");
//		return false;
//	}
//
//	if(sqlite3_bind_text(m_pStm, 10, event->m_result.c_str(), -1, SQLITE_TRANSIENT)!= SQLITE_OK) {
//		syslog(LOG_ERR, "[SQL] Failed to bind text 3");
//		return false;
//	}
//
//
//	int s;
//	if ((s = sqlite3_step(m_pStm)) != SQLITE_DONE) {
//		syslog(LOG_ERR, "[SQL] Failed to step: %d %s", s, getLastError());
//		return false;
//	}
//	sqlite3_reset(m_pStm);
//	sqlite3_clear_bindings(m_pStm);
//	return true;
//}
bool EventLoggerThread::Insert(DBEventCommon* event)
{
	//bind parameters first
	sqlite3_bind_int(m_pStm, 1, event->getTypeId());
	sqlite3_bind_int(m_pStm, 2, event->getChannelId());
	sqlite3_bind_text(m_pStm, 3, Utils::TimeToString(event->getRegisterTimeDate()).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(m_pStm, 4, event->getArgument1().c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(m_pStm, 5, event->getArgument2().c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(m_pStm, 6, event->getArgument3().c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(m_pStm, 7, event->getArgument4().c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(m_pStm) != SQLITE_DONE) {
		syslog(LOG_ERR, "[SQL] Failed to step: %s", getLastError());
		return false;
	}

	sqlite3_reset(m_pStm);
	sqlite3_clear_bindings(m_pStm);
	return true;
}
