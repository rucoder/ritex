/*
 * LoggerThread.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef LOGGERTHREAD_H_
#define LOGGERTHREAD_H_

#include "Thread.h"

#include <queue>
#include <string>
#include <sqlite3.h>
#include <syslog.h>

template <typename T>
class LoggerThread: public Thread {
private:
	// sqlite3
	std::string m_sDbName;
	sqlite3* m_pDb;
	// queues
	std::queue<T> m_queue;
	pthread_cond_t m_condProcessQueue;
	pthread_mutex_t m_queueMutex;
	void CloseDb() {
		if(m_pStm) {
			sqlite3_finalize(m_pStm);
			m_pStm = NULL;
		}
		if(m_pDb) {
			sqlite3_close(m_pDb);
			m_pDb = NULL;
		}
	}
protected:
	std::string m_sSqlQuery;
	sqlite3_stmt* m_pStm; //insert statement
protected:
	LoggerThread()
		: m_pDb(NULL), m_pStm(NULL)
	{

	};
	virtual bool Insert(T data) = 0;
	virtual void* Run()
	{
		T pData;
		while(true) {

			pthread_mutex_lock(&m_queueMutex);
			while(m_queue.empty())
				pthread_cond_wait(&m_condProcessQueue, &m_queueMutex);

			pData = m_queue.front();
			m_queue.pop();

			pthread_mutex_unlock(&m_queueMutex);

			//write data to DB
			if(pData) {
				if(!Insert(pData))
				{
					syslog(LOG_ERR, "[SQL] Failed to insert data!");
				}
				delete pData;
			}
		}
		return NULL;
	}

	const char* getLastError() {
		return sqlite3_errmsg(m_pDb);
	}

public:
	virtual ~LoggerThread() {
		Cancel();
		Join();
		pthread_cond_destroy(&m_condProcessQueue);
		pthread_mutex_destroy(&m_queueMutex);
		while(!m_queue.empty()) {
			T pData = m_queue.front();
			m_queue.pop();
			if(pData) {
				delete pData;
			}
		}
		CloseDb();
	};
	LoggerThread(std::string db_name)
		: m_sDbName(db_name), m_pDb(NULL), m_pStm(NULL) {

	}

	virtual bool Create(bool createDetached = false)
	{
		int rc;
		//try open DB
		rc = sqlite3_open_v2(m_sDbName.c_str(), &m_pDb, SQLITE_OPEN_READWRITE, NULL);

	    if(rc != SQLITE_OK)
	    {
	    	syslog(LOG_ERR,"[SQL] couldn't open DB: %s\n", m_sDbName.c_str());
	    	CloseDb();
	    	return false;
	    }
	    rc = sqlite3_prepare_v2(m_pDb, m_sSqlQuery.c_str(),-1, &m_pStm, NULL);

	    if(rc != SQLITE_OK)
	    {
	    	syslog(LOG_ERR,"[SQL] couldn't prepare 'insert event' statement for: %s : error %s\n", m_sDbName.c_str(), sqlite3_errmsg(m_pDb));
	    	CloseDb();
	    	return false;
	    }

	    //TODO: check for errors and cleanup
		pthread_cond_init(&m_condProcessQueue, NULL);
		pthread_mutex_init(&m_queueMutex, NULL);

		return Thread::Create(createDetached);
	}

	bool EnqueData(T event) {
		//TODO: check return values
		pthread_mutex_lock(&m_queueMutex);
		m_queue.push(event);
		pthread_mutex_unlock(&m_queueMutex);
		pthread_cond_signal(&m_condProcessQueue);
		return true;

	};
};
#endif /* LOGGERTHREAD_H_ */
