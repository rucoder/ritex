/*
 * EventLoggerThread.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef EVENTLOGGERTHREAD_H_
#define EVENTLOGGERTHREAD_H_

#include "Thread.h"
#include "DBEventCommand.h"
#include "DBEventCommon.h"
#include <queue>
#include <string>
#include <sqlite3.h>


class EventLoggerThread: public Thread {
private:
	// sqlite3
	std::string m_sDbName;
	sqlite3* m_pDb;
	sqlite3_stmt* m_pCmdStm; //insert statement for Command events
	sqlite3_stmt* m_pEventStm; //insert statement for regular events
	// queues
	std::queue<DBEventCommand*> m_cmdQueue;
	std::queue<DBEventCommon*> m_eventQueue;
	pthread_cond_t m_condProcessQueue;
	pthread_mutex_t m_queueMutex;
	void CloseDb();
	bool InsertEvent(DBEventCommon* event);
	bool InsertCommand(DBEventCommand* event);
protected:
	EventLoggerThread();
	virtual void* Run();
public:
	EventLoggerThread(std::string db_name);
	virtual ~EventLoggerThread();
	virtual bool Create(bool createDetached = false);
	bool EnqueCommandEvent(DBEventCommand* event);
	bool EnqueCommonEvent(DBEventCommon* event);
};

#endif /* EVENTLOGGERTHREAD_H_ */
