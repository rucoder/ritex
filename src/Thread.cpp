/*
 * Thread.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "Thread.h"

void Thread::thread_oncancel_function(void* param)
{
	Thread* pThread = reinterpret_cast<Thread*>(param);
	pThread->OnCancel();
}


void* Thread::thread_function(void* param)
{
	Thread* pThread = reinterpret_cast<Thread*>(param);
	void * ret;
	pthread_cleanup_push(thread_oncancel_function, param);
	ret = pThread->Run();
	pthread_cleanup_pop(0);
	return ret;
}


Thread::Thread()
	: m_hHandle(-1), m_isJoinable(true)
{
}



Thread::~Thread() {
	Cancel();
	Join();
}

bool Thread::Create(bool createDetached)
{
	pthread_attr_t* pAttr = NULL;

	m_isJoinable = !createDetached;

	if(createDetached) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pAttr = &attr;
	}

	int retVal = pthread_create(&m_hHandle, pAttr, thread_function, this);

	if (pAttr != NULL) {
		pthread_attr_destroy(pAttr);
	}

	return retVal == 0;
}
bool Thread::Join()
{
	if(m_isJoinable) {
		void* threadRetVal;
		int retVal = pthread_join(m_hHandle, &threadRetVal);
		return retVal == 0;
	}
	return false;
}
bool Thread::Cancel()
{
	pthread_cancel(m_hHandle);
	return true;
}

void Thread::OnCancel()
{

}


