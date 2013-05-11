/*
 * DataLoggerThread.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "DataLoggerThread.h"
#include "Utils.h"
#include <syslog.h>
#include <assert.h>

DataLoggerThread::DataLoggerThread(std::string db_name)
	: LoggerThread<DBDataPacket*>(db_name)
{
	// int,int,text (date-time),float
	m_sSqlQuery = "INSERT INTO tblparamdata (ChannelId, ParamId, RegisterDate, Value)  VALUES(?,?,?,?)";
}

bool DataLoggerThread::Insert(DBDataPacket* event)
{
	syslog(LOG_ERR, "[SQL] Inserting data: chId=%d, paramId=%d, td=%lu %s val=%g",event->getChannelId(), event->getParamId(),
	 event->getRegisterDate(), TimeToString(event->getRegisterDate()).c_str(), event->getValue());
	//bind parameters first
	if(sqlite3_bind_int(m_pStm, 1, event->getChannelId()) != SQLITE_OK) {
		syslog(LOG_ERR, "[SQL] Failed to bind int 1");
		return false;
	}
		
	if(sqlite3_bind_int(m_pStm, 2, event->getParamId())!= SQLITE_OK) {
		syslog(LOG_ERR, "[SQL] Failed to bind int 2");
		return false;
	}
	if(sqlite3_bind_text(m_pStm, 3, TimeToString(event->getRegisterDate()).c_str(), -1, SQLITE_TRANSIENT)!= SQLITE_OK) {
		syslog(LOG_ERR, "[SQL] Failed to bind text");
		return false;
	}
	if(sqlite3_bind_double(m_pStm, 4, event->getValue())!= SQLITE_OK) {
		syslog(LOG_ERR, "[SQL] Failed to bind double");
		return false;
	}

	if (sqlite3_step(m_pStm) != SQLITE_DONE) {
		syslog(LOG_ERR, "[SQL] Failed to step");
		return false;
	}
	sqlite3_reset(m_pStm);
	sqlite3_clear_bindings(m_pStm);
	return true;
}



