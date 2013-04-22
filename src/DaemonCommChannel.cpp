/*
 * DaemonCommChannel.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DaemonCommChannel.h"
//socket
#include <sys/un.h>
#include <sys/socket.h>
//errno
#include <errno.h>
// nanosleep
#include <time.h>
// close()
#include <unistd.h>

DaemonCommChannel::DaemonCommChannel()
	: m_fd(-1)
{
	// TODO Auto-generated constructor stub

}

DaemonCommChannel::~DaemonCommChannel() {
	close();
}

int DaemonCommChannel::open(std::string socketName)
{
	struct sockaddr_un addr;

	// 0.1 sec sleep. TODO: make timeout configurable
	struct timespec tim;
	tim.tv_sec = 0;
	tim.tv_nsec = 100000000;
	int connectResult;

	m_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);

	if(m_fd == -1)
		return errno;

	// create abstract socket name
	::memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	::strncpy(&addr.sun_path[1], socketName.c_str(), sizeof(addr.sun_path) - 2);

	// wait for 5 sec at most
	for(int i = 0; i < 50; i++) {
		connectResult = ::connect(m_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
		if (connectResult == -1) {
			::nanosleep(&tim, NULL);
		} else {
			break;
		}
	}

	if (connectResult == -1) {
		close();
		return errno;
	}

	return 0;
}

int DaemonCommChannel::close()
{
	if(m_fd > 0) {
		::close(m_fd);
	}
}

int DaemonCommChannel::send(unsigned char* buffer, int length)
{
	unsigned char* envelope = new unsigned char[length + sizeof(int)];
	if (envelope == NULL)
		return -1;
	::memcpy(envelope, &length, sizeof(int));
	::memcpy(envelope + sizeof(int), buffer, length);
	return ::send(m_fd,envelope,length + sizeof(int),0);
}

int DaemonCommChannel::recv(unsigned char* buffer, int length)
{
#if 0
	unsigned char* envelope = new unsigned char[length + sizeof(int)];
	if (envelope == NULL)
		return -1;
	::memcpy(envelope, &length, sizeof(int));
	::memcpy(envelope + sizeof(int), buffer, length);
	::send(m_fd,envelope,length + sizeof(int),0);
#endif
	return ::recv(m_fd, buffer, length, 0);
}

