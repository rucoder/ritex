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
	syslog(LOG_ERR, "[SQL] Inserting data...");
	//bind parameters first
	sqlite3_bind_int(m_pStm, 1, event->getChannelId());
	sqlite3_bind_int(m_pStm, 2, event->getParamId());
	sqlite3_bind_text(m_pStm, 3, Utils::TimeToString(event->getRegisterDate()).c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_double(m_pStm, 4, event->getValue());

	//now execute query

}



