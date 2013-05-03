/*
 * Thread.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>

class Thread {
private:
	pthread_t m_hHandle;
	bool m_isJoinable;
	static void* thread_function(void* param);
	static void  thread_oncancel_function(void* param);
protected:
	virtual void* Run() = 0;
	virtual void OnCancel();
public:
	Thread();
	virtual ~Thread();
	virtual bool Create(bool createDetached = false);
	bool Join();
	bool Cancel();
};

#endif /* THREAD_H_ */
