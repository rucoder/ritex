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
	static void* thread_function(void* param);
protected:
	virtual void* Run() = 0;
public:
	Thread();
	virtual ~Thread();
	bool Create(bool createDetached = false);
	bool Join();
	bool Cancel();
};

#endif /* THREAD_H_ */
