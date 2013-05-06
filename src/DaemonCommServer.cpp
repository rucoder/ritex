/*
 * DaemonCommServer.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "DaemonCommServer.h"
#include "CmdLineParser.h"
#include "CmdLineCommand.h"
#include "DeviceCommand.h"

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
#include <unistd.h>


#include "Utils.h"
DaemonCommServer::DaemonCommServer(Device* pDevice, IAdapter* pAdapter)
	: Thread(), m_pDevice(pDevice), m_pAdapter(pAdapter), m_connectSock(-1), m_clientSock(-1), m_canProcessCommands(true)
{

}

DaemonCommServer::~DaemonCommServer() {
	OnCancel();
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

	syslog(LOG_ERR, "Opening server socket.. [%s] ", name);


	if (bind(m_connectSock, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
		syslog(LOG_ERR, "bind");
		close(m_connectSock);
		m_connectSock = -1;
		return false;
	}
	if (listen(m_connectSock, 5) == -1) {
		syslog(LOG_ERR, "listen");
		close(m_connectSock);
		m_connectSock = -1;
		return false;
	}

	pthread_cond_init(&m_canProcessCommandsCond, NULL);
	pthread_mutex_init(&m_mutex,NULL);

	m_pDevice->SetDeviceStatusListener(this);

	return Thread::Create(createDetached);
}

void* DaemonCommServer::Run() {

	int numRead;

	CmdLineParser* pParser = new CmdLineParser();

	for (;;) {
		/* Handle client connections iteratively */

		/* Accept a connection. The connection is returned on a new
		 *socket, 'm_clientSock'; the listening socket ('m_connectSock') remains open
		 *and can be used to accept further connections.
		 */
		syslog(LOG_ERR, "Waitign for client connection...");
		m_clientSock = accept(m_connectSock, NULL, NULL);
		if (m_clientSock == -1) {
			syslog(LOG_ERR, "accept");
			return NULL;
		}

		/*
		 * Command handling loop
		 */
		syslog(LOG_ERR, "Client connected. m_clientSock=0x%X", m_clientSock);
		do {
			int length;
			unsigned char* rawData;
			numRead = recv(m_clientSock, &length, 4, 0); //get the length

			syslog(LOG_ERR, "Got packet. length=%d", length);

			if(numRead < 0) {
				m_clientSock = -1;
				break;
			}

			if(numRead == 4) {
				rawData = new unsigned char[length];
				numRead = recv(m_clientSock, rawData, length, 0); //get the data

				syslog(LOG_ERR, "Got packet data read. =%d expected=%d", numRead, length);

				syslog(LOG_ERR, "data: %s", rawData);


				//good packet
				if(numRead == length) {
					syslog(LOG_ERR, "--- 0");
					pParser->SetCmdLine((char*)rawData);
					syslog(LOG_ERR, "--- 0.1");

					//valid command line ?
					if (pParser->Parse()) {
						syslog(LOG_ERR, "--- 1");
						CmdLineCommand* pCommand = pParser->GetCommand();
						if(pCommand && pCommand->Compile(m_pAdapter->GetAdditionalParameterMap())) {
							syslog(LOG_ERR, "--- 2");
							DeviceCommand* pDevCmd = m_pDevice->CreateCommand(pCommand);
							if (pDevCmd) {
								int s;
								syslog(LOG_ERR, "--- 3");

								syslog(LOG_ERR, "Executing...");

								s = pthread_mutex_lock(&m_mutex);
								if( s != 0) {
									syslog(LOG_ERR, "~~~~~~~~~~MUTEX s=%d",s);
								}

								m_canProcessCommands = false;
								s = pthread_mutex_unlock(&m_mutex);
								if( s != 0) {
									syslog(LOG_ERR, "~~~~~~~~~~MUTEX s=%d",s);
								}


								pDevCmd->AddResultListener(this);
								pDevCmd->Execute();
								syslog(LOG_ERR, "Executing. WAIT->>");

								s = pthread_mutex_lock(&m_mutex);
								syslog(LOG_ERR, "Executing. WAIT: locked %d", s);

								while(!m_canProcessCommands) {
									syslog(LOG_ERR, "Executing. WAIT: wait on cond");
									pthread_cond_wait(&m_canProcessCommandsCond, &m_mutex);
								}
								s = pthread_mutex_unlock(&m_mutex);
								if( s != 0) {
									syslog(LOG_ERR, "~~~~~~~~~~MUTEX s=%d",s);
								}

								syslog(LOG_ERR, "Executing. WAIT-<<");

								delete pDevCmd;
							}
						}
					}
					continue;
				}
				break;
			}
			break;

		} while(1);
		//TODO: handle error better
	}
	syslog(LOG_ERR, "DaemonCommServer::Run(): DONE");
	return NULL;
}

void DaemonCommServer::OnResultReady(DeviceCommand* pCmd)
{
	syslog(LOG_ERR,"DaemonCommServer::OnResultReady->>");

	int length = pCmd->getRawResultLength();

	syslog(LOG_ERR, "Sending result: length: %d", length);

	unsigned char* envelope = new unsigned char[length + sizeof(int)];
	//if (envelope == NULL)
	//	return -1;
	::memcpy(envelope, &length, sizeof(int));
	if(length > 0) {
		::memcpy(envelope + sizeof(int), pCmd->getRawResult(), length);
	}

	syslog(LOG_ERR, "Calling send()");
	send(m_clientSock,envelope,length + sizeof(int),0);
	delete [] envelope;
	//delete pCmd;
	syslog(LOG_ERR, "Calling send(): afetr");

	int s = pthread_mutex_lock(&m_mutex);
	if( s != 0) {
		syslog(LOG_ERR, "~~~~~~~~~~MUTEX s=%d",s);
	}

	syslog(LOG_ERR, "Calling send(): locked");

	m_canProcessCommands = true;
	s = pthread_mutex_unlock(&m_mutex);
	if( s != 0) {
		syslog(LOG_ERR, "~~~~~~~~~~MUTEX s=%d",s);
	}

	syslog(LOG_ERR, "Calling send(): unlocked");
	pthread_cond_signal(&m_canProcessCommandsCond);
	syslog(LOG_ERR, "Calling send(): after 1");

	syslog(LOG_ERR,"DaemonCommServer::OnResultReady-<<");
}

void DaemonCommServer::OnDeviceStatusChanged(eDeviceStaus status)
{
	syslog(LOG_ERR, "OnDeviceStatusChanged: %d", status);
	if (status == DEVICE_STATUS_EXIT) {
		Cancel();
	}
}

void DaemonCommServer::OnCancel() {
	syslog(LOG_ERR, "OnCancel!!!!:");

	if(m_clientSock != -1) {
		m_clientSock = -1;
		close(m_clientSock);
	}
	if(m_connectSock != -1) {
		m_connectSock = -1;
		close(m_connectSock);
	}
	pthread_cond_destroy(&m_canProcessCommandsCond);
	pthread_mutex_destroy(&m_mutex);
}


