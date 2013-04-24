/*
 * DaemonCommChannel.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DAEMONCOMMCHANNEL_H_
#define DAEMONCOMMCHANNEL_H_

#include <string>

class DaemonCommChannel {
protected:
	int m_fd; // socket file descriptor
public:
	DaemonCommChannel();
	virtual ~DaemonCommChannel();
	int open(std::string socketName);
	int close();
	bool isOpened() { return m_fd != -1; };
	int send(unsigned char* buffer, int length);
	int recv(unsigned char* buffer, int length);
};

#endif /* DAEMONCOMMCHANNEL_H_ */
