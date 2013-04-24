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

EventLoggerThread::EventLoggerThread() {
	// TODO Auto-generated constructor stub

}

EventLoggerThread::EventLoggerThread(std::string db_name)
	: m_sDbName(db_name), m_pDb(NULL), m_pCmdStm(NULL), m_pEventStm(NULL)
{

}

EventLoggerThread::~EventLoggerThread() {
	Cancel();
	Join();
	pthread_cond_destroy(&m_condProcessQueue);
	pthread_mutex_destroy(&m_queueMutex);
	CloseDb();
}

void EventLoggerThread::CloseDb()
{
	if(m_pEventStm) {
		sqlite3_finalize(m_pEventStm);
		m_pEventStm = NULL;
	}
	if(m_pCmdStm) {
		sqlite3_finalize(m_pCmdStm);
		m_pCmdStm = NULL;
	}
	if(m_pDb) {
		sqlite3_close(m_pDb);
		m_pDb = NULL;
	}
}

bool EventLoggerThread::Create(bool createDetached)
{
	int rc;
	//try open DB
	rc = sqlite3_open_v2(m_sDbName.c_str(), &m_pDb, SQLITE_OPEN_READWRITE, NULL);

    if(rc != SQLITE_OK)
    {
    	syslog(LOG_ERR,"[SQL] couldn't open DB\n");
    	CloseDb();
    	return false;
    }
//insert  into tbleventbus (TypeId, ChannelId, RegisterDate, Argument1, Argument2, Argument3, Argument4) values(1,  2,\"c\", \"X\", \"X\", \"X\", \"X\")
    rc = sqlite3_prepare_v2(m_pDb,
    		"insert  into tbleventbus (TypeId, ChannelId, RegisterDate, Argument1, Argument2, Argument3, Argument4) values(?,?,?,?,?,?,?)"
    		,-1, &m_pEventStm, NULL);



    if(rc != SQLITE_OK)
    {
    	syslog(LOG_ERR,"[SQL] couldn't prepare 'insert event' statement\n");
    	CloseDb();
    	return false;
    }

    assert(sqlite3_bind_parameter_count(m_pEventStm) == 7);

//insert  into tblcommand (CommandId, Source, State, Type, Message, Priority, DeviceId,ArrivalDate,EndExecDate,Result) values(1,  "c", 1, 1, "X", 1,1,"X","X","X")
    rc = sqlite3_prepare_v2(m_pDb,
    		"insert  into tblcommand (CommandId, Source, State, Type, Message, Priority, DeviceId,ArrivalDate,EndExecDate,Result) values(?,  ?, ?, ?, ?, ?,?,?,?,?)",
    		-1, &m_pCmdStm, NULL);

    if(rc != SQLITE_OK)
    {
    	syslog(LOG_ERR,"[SQL] couldn't prepare 'insert command' statement\n");
    	CloseDb();
    	return false;
    }

    assert(sqlite3_bind_parameter_count(m_pCmdStm) == 10);


	pthread_cond_init(&m_condProcessQueue, NULL);
	pthread_mutex_init(&m_queueMutex, NULL);

	return Thread::Create(createDetached);
}

bool EventLoggerThread::EnqueCommandEvent(DBEventCommand* event)
{
	//TODO: check return values
	pthread_mutex_lock(&m_queueMutex);
	m_cmdQueue.push(event);
	pthread_mutex_unlock(&m_queueMutex);
	pthread_cond_signal(&m_condProcessQueue);
	return true;
}
bool EventLoggerThread::EnqueCommonEvent(DBEventCommon* event)
{
	//TODO: check return values
	pthread_mutex_lock(&m_queueMutex);
	m_eventQueue.push(event);
	pthread_mutex_unlock(&m_queueMutex);
	pthread_cond_signal(&m_condProcessQueue);
	return true;
}

bool EventLoggerThread::InsertEvent(DBEventCommon* event)
{
	//bind parameters first
	sqlite3_bind_int(m_pEventStm, 1, event->getTypeId());
	sqlite3_bind_int(m_pEventStm, 2, event->getChannelId());
	sqlite3_bind_text(m_pEventStm, 3, Utils::TimeToString(event->getRegisterTimeDate()).c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pEventStm, 4, event->getArgument1().c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pEventStm, 5, event->getArgument2().c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pEventStm, 6, event->getArgument3().c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(m_pEventStm, 7, event->getArgument4().c_str(), -1, SQLITE_STATIC);

	//now execute query
	return true;
}

bool EventLoggerThread::InsertCommand(DBEventCommand* event)
{
	//bind parameters first
	return true;
}

void* EventLoggerThread::Run()
{
	DBEventCommand* pCmd;
	DBEventCommon* pEvent;
	while(true) {
		pthread_mutex_lock(&m_queueMutex);
		while(m_cmdQueue.empty() && m_eventQueue.empty())
			pthread_cond_wait(&m_condProcessQueue, &m_queueMutex);

		if(!m_cmdQueue.empty()) {
			pCmd = m_cmdQueue.front();
			m_cmdQueue.pop();
		}

		if(!m_eventQueue.empty()) {
			pEvent = m_eventQueue.front();
			m_eventQueue.pop();
		}
		pthread_mutex_unlock(&m_queueMutex);

		//write event to DB
		if(pEvent) {
			InsertEvent(pEvent);
		}

		//write command event to DB
		if(pCmd) {
			InsertCommand(pCmd);
		}
	}
	return NULL;
}

