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
#include <assert.h>

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
	std::string m_TAG;
private:
	sqlite3_stmt* m_pStm; //insert statement
protected:
	LoggerThread()
		: m_pDb(NULL), m_pStm(NULL)
	{

	};
	virtual bool BindParams(T data) = 0;
	virtual void* Run()
	{
		T pData;
		while(true) {

			timespec tm;

			tm.tv_sec = 2;
			tm.tv_nsec = 0;

			pthread_mutex_lock(&m_queueMutex);
			while(m_queue.empty())
				pthread_cond_timedwait(&m_condProcessQueue, &m_queueMutex, &tm);

			pData = m_queue.front();
			m_queue.pop();

			pthread_mutex_unlock(&m_queueMutex);

			//write data to DB
			if(pData) {
				if(BindParams(pData)) {
					int error = sqlite3_step(m_pStm);
					if (error == SQLITE_DONE) {
						delete pData;
						pData = NULL;
					} else if(error == SQLITE_BUSY) {
						Log("DB is busy. will try again");
					}
				}
				//put data back to queue but not notify.
				if(pData) {
					EnqueData(pData, false);
					Log("Couldn't insert data. ERROR: [%d-%d] %s", getLastError(), getLastExtError(),  getLastErrorMsg());
				}
				//reset statement and clear bindings in any case
				sqlite3_reset(m_pStm);
				sqlite3_clear_bindings(m_pStm);
			}
		}
		return NULL;
	}

	const char* getLastErrorMsg() {
		return sqlite3_errmsg(m_pDb);
	}

	int getLastError() {
		return sqlite3_errcode(m_pDb);
	}

	int getLastExtError() {
		return sqlite3_extended_errcode(m_pDb);
	}
	void flush() {
		//just in case we terminated thread somewhere in between
		sqlite3_reset(m_pStm);
		sqlite3_clear_bindings(m_pStm);
		while(!m_queue.empty()) {
			T pData = m_queue.front();
			m_queue.pop();
			if(pData) {
				if(BindParams(pData))
				{
					if (sqlite3_step(m_pStm) != SQLITE_DONE) {
						Log("Failed to insert data from flush(): STEP : ERROR: %s!", getLastError());
					}

				} else {
					Log("Failed to insert data from flush(): BIND : ERROR: %s!", getLastError());
				}
				delete pData;
				sqlite3_reset(m_pStm);
				sqlite3_clear_bindings(m_pStm);
			}
		}
	}

	bool Bind(int index, int value) {
		if(sqlite3_bind_int(m_pStm, index, value) != SQLITE_OK) {
			Log("Couldn't bind int: index %d", index);
			return false;
		}
		return true;
	}

	bool Bind(int index, double value) {
		if(sqlite3_bind_double(m_pStm, index, value) != SQLITE_OK) {
			Log("Couldn't bind double: index %d", index);
			return false;
		}
		return true;
	}

	bool Bind(int index, const std::string value) {
		if(sqlite3_bind_text(m_pStm, index, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
			Log("Couldn't bind text: index %d", index);
			return false;
		}
		return true;
	}

	void Log(std::string format, ...) {
		std::string fullFormat = "[SQL] " + m_TAG + " :" + format;
		va_list args;
		va_start(args, format);
		vsyslog(LOG_ERR, fullFormat.c_str(), args);
		va_end(args);
	}

public:
	virtual ~LoggerThread() {
		Log("~LoggerThread->>");
		Cancel();
		Join();
		pthread_cond_destroy(&m_condProcessQueue);
		pthread_mutex_destroy(&m_queueMutex);
		flush();
		assert(m_queue.empty());
		CloseDb();
		Log("~LoggerThread-<<");
	};
	LoggerThread(std::string db_name, std::string tag)
		: m_sDbName(db_name), m_pDb(NULL), m_TAG(tag), m_pStm(NULL) {

	}

	virtual bool Create(bool createDetached = false)
	{
		int rc;
		//try open DB
		rc = sqlite3_open_v2(m_sDbName.c_str(), &m_pDb, SQLITE_OPEN_READWRITE, NULL);

	    if(rc != SQLITE_OK)
	    {
	    	Log("couldn't open DB: %s\n", m_sDbName.c_str());
	    	CloseDb();
	    	return false;
	    }
	    rc = sqlite3_prepare_v2(m_pDb, m_sSqlQuery.c_str(),-1, &m_pStm, NULL);

	    if(rc != SQLITE_OK)
	    {
	    	Log("couldn't prepare 'insert event' statement for: %s : error %s\n", m_sDbName.c_str(), sqlite3_errmsg(m_pDb));
	    	CloseDb();
	    	return false;
	    }

	    //TODO: check for errors and cleanup
		pthread_cond_init(&m_condProcessQueue, NULL);
		pthread_mutex_init(&m_queueMutex, NULL);

		return Thread::Create(createDetached);
	}

	bool EnqueData(T event, bool signal = true) {
		//TODO: check return values
		pthread_mutex_lock(&m_queueMutex);
		m_queue.push(event);
		pthread_mutex_unlock(&m_queueMutex);
		if(signal)
			pthread_cond_signal(&m_condProcessQueue);
		return true;

	};
};
#endif /* LOGGERTHREAD_H_ */
