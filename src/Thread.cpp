/*
 * Thread.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "Thread.h"

void* Thread::thread_function(void* param)
{
	Thread* pThread = reinterpret_cast<Thread*>(param);
	return pThread->Run();
}


Thread::Thread() {
	// TODO Auto-generated constructor stub

}

Thread::~Thread() {
	// TODO Auto-generated destructor stub
}

bool Thread::Create(bool createDetached)
{
	pthread_attr_t* pAttr = NULL;

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
	void* threadRetVal;
	int retVal = pthread_join(m_hHandle, &threadRetVal);
	return retVal == 0;
}
bool Thread::Cancel()
{
	return false;
}


