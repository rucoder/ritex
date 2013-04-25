/*
 * DaemonCommServer.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "DaemonCommServer.h"

//socket
#include <sys/un.h>
#include <sys/socket.h>

//syslog
#include <syslog.h>

//assert
#include <assert.h>

//close
#include <stdlib.h>
#include <stdio.h>

DaemonCommServer::DaemonCommServer(Device* pDevice)
	: Thread(), m_pDevice(pDevice), m_connectSock(-1), m_clientSock(-1)
{

}

DaemonCommServer::~DaemonCommServer() {
	// TODO Auto-generated destructor stub
}

bool DaemonCommServer::Create(bool createDetached )
{
	struct sockaddr_un addr;

	m_connectSock = socket(AF_UNIX, SOCK_STREAM, 0);

	if (m_connectSock == -1) {
		return false;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	//TODO: fix it
	char name[32];
	snprintf(name, 32,"Ritex_socket%d", m_pDevice->getDeviceId());
	/// end

	strncpy(&addr.sun_path[1], name, sizeof(addr.sun_path) - 2);

	if (bind(m_connectSock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
		syslog(LOG_ERR, "bind");
		//TODO: close(m_connectSock);
		m_connectSock = -1;
		return false;
	}
	if (listen(m_connectSock, 5) == -1) {
		syslog(LOG_ERR, "listen");
		//TODO: close(m_connectSock);
		m_connectSock = -1;
		return false;
	}

	return Thread::Create(createDetached);
}

void* DaemonCommServer::Run() {

	int numRead;

	for (;;) {
		/* Handle client connections iteratively */

		/* Accept a connection. The connection is returned on a new
		 *socket, 'm_clientSock'; the listening socket ('m_connectSock') remains open
		 *and can be used to accept further connections.
		 */
		m_clientSock = accept(m_connectSock, NULL, NULL);
		if (m_clientSock == -1) {
			syslog(LOG_ERR, "accept");
			return NULL;
		}

		syslog(LOG_ERR, "");
		/*
		 * Command handling loop
		 */
		do {
			int length;
			unsigned char* rawData;
			numRead = recv(m_clientSock, &length, 4, 0); //get the length

			syslog(LOG_ERR, "Got packet. length=%d", length);


			if(numRead == 4) {
				rawData = new unsigned char[length];
				numRead = recv(m_clientSock, rawData, length, 0); //get the data

				syslog(LOG_ERR, "Got packet data read. =%d", numRead);

				//good packet
				if(numRead == length) {

					DeviceCommand* pCommand = m_pDevice->CreateCommand(rawData, length);


					if(pCommand) {
						syslog(LOG_ERR, "Executing...");
						//paranoid check
						assert(pCommand->isHWCommand());
						//execute and write back the result
						//m_pDevice->Execute(pCommand);
						pCommand->Execute();
						//send result back to client
						//create envelope expected by DaemonCommChannel

						length = pCommand->getRawResultLength();

						syslog(LOG_ERR, "Sending result: length: %d", length);


						unsigned char* envelope = new unsigned char[length + sizeof(int)];
						//if (envelope == NULL)
						//	return -1;
						::memcpy(envelope, &length, sizeof(int));
						if(length > 0) {
							::memcpy(envelope + sizeof(int), pCommand->getRawResult(), length);
						}

						syslog(LOG_ERR, "Calling send()");
						send(m_clientSock,envelope,length + sizeof(int),0);
						delete [] envelope;
						delete pCommand;
					} else {
						syslog(LOG_ERR,"Command not found");
						//should not really ever happen. for debugging only
//						unsigned char* envelope = new unsigned char[2 * sizeof(int)];
//						length = 2 * sizeof(int);
//						//if (envelope == NULL)
//						//	return -1;
//						::memcpy(envelope, &length, sizeof(int));
//
//						length = -1;
//						if(length > 0) {
//							::memcpy(envelope + sizeof(int), pCommand->getRawResult(), length);
//						}
//
//						send(m_clientSock,envelope,length,0);
//						delete envelope;
					}
					continue;
				}
				break;
			}
			break;

		} while(1);
		//TODO: handle error better
	}

	return NULL;
}
