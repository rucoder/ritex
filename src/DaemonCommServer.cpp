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
#include "Log.h"

//assert
#include <assert.h>

//close
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <errno.h>

#include "Utils.h"
DaemonCommServer::DaemonCommServer(Device* pDevice, IAdapter* pAdapter) :
		Thread(), m_pDevice(pDevice), m_pAdapter(pAdapter), m_connectSock(-1), m_clientSock(
				-1), m_canProcessCommands(true) {

}

DaemonCommServer::~DaemonCommServer() {
}

bool DaemonCommServer::Create(bool createDetached) {
	struct sockaddr_un addr;

	m_connectSock = socket(AF_UNIX, SOCK_STREAM, 0);

	if (m_connectSock == -1) {
		return false;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	//TODO: fix it
	char name[32];
	snprintf(name, 32, "Ritex_socket%d", m_pDevice->getDeviceId());
	/// end

	strncpy(&addr.sun_path[1], name, sizeof(addr.sun_path) - 2);

	Log( "DaemonCommServer: Opening server socket.. [%s] ", name);

	if (bind(m_connectSock, (struct sockaddr *) &addr, _STRUCT_OFFSET (struct sockaddr_un, sun_path) + strlen(name) + 1) == -1) {
		Log( "DaemonCommServer: bind() error");
		close(m_connectSock);
		m_connectSock = -1;
		return false;
	}
	if (listen(m_connectSock, 5) == -1) {
		Log( "DaemonCommServer listen() error");
		close(m_connectSock);
		m_connectSock = -1;
		return false;
	}

	pthread_cond_init(&m_canProcessCommandsCond, NULL);
	pthread_mutex_init(&m_mutex, NULL);

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
#ifndef KSU_EMULATOR
		Log( "DaemonCommServer: Waitign for client connection...");
		m_clientSock = accept(m_connectSock, NULL, NULL);
		if (m_clientSock == -1) {
			Log( "DaemonCommServer: accept() error");
			return NULL;
		}
#endif

		/*
		 * Command handling loop
		 */
		Log( "DaemonCommServer: Client connected. m_clientSock=0x%X",
				m_clientSock);
		do {
#ifdef KSU_EMULATOR
			int length;
			numRead = 4;
			length = 4;
			char* rawData = new char[1024];
#else
			int length;
			unsigned char* rawData;
			numRead = recv(m_clientSock, &length, sizeof(int), MSG_WAITALL); //get the length

			Log( "DaemonCommServer: Got packet. length=%d numRead=%d", length, numRead);

			if(numRead < 0) {
				m_clientSock = -1;
				SendResponseToClient(
						"7;[SERVER] error getting  content length\n");
				break;
			}
#endif

			if (numRead == sizeof(int)) {
#ifdef KSU_EMULATOR
				printf("input command>");
				gets(rawData);
#else
				rawData = new unsigned char[length];
				numRead = recv(m_clientSock, rawData, length, MSG_WAITALL); //get the data
#endif

#ifdef __DEBUG__
				Log(
						"DaemonCommServer: Got packet data read. =%d expected=%d",
						numRead, length);
#endif

				Log(
						"DaemonCommServer: command line from client: %s",
						rawData);

				//good packet
				if (numRead == length) {

#ifdef __DEBUG__
					Log( "--- 0");
#endif
					pParser->SetCmdLine((char*) rawData);
#ifdef __DEBUG__
					Log( "--- 0.1");
#endif

					//valid command line ?
					if (pParser->Parse()) {
#ifdef __DEBUG__
						Log( "--- 1");
#endif
						CmdLineCommand* pCommand = pParser->GetCommand();
						if (pCommand
								&& pCommand->Compile(
										m_pAdapter->GetAdditionalParameterMap())) {
#ifdef __DEBUG__
							Log( "--- 2");
#endif
							DeviceCommand* pDevCmd = m_pDevice->CreateCommand(
									pCommand);
							if (pDevCmd) {
								int s;
#ifdef __DEBUG__
								Log( "--- 3");
#endif

								Log(
										"DaemonCommServer: Executing external command...");

								s = pthread_mutex_lock(&m_mutex);
								if (s != 0) {
									Log( "~~~~~~~~~~MUTEX s=%d", s);
								}

								m_canProcessCommands = false;
								s = pthread_mutex_unlock(&m_mutex);
								if (s != 0) {
									Log( "~~~~~~~~~~MUTEX s=%d", s);
								}

								pDevCmd->AddResultListener(this);
								if (pDevCmd->Execute()) {
									Log(
											"DaemonCommServer: Executing. WAIT->>");

									s = pthread_mutex_lock(&m_mutex);
									Log(
											"DaemonCommServer: Executing. WAIT: locked %d",
											s);

									while (!m_canProcessCommands) {
										Log(
												"Executing. WAIT: wait on cond");
										pthread_cond_wait(
												&m_canProcessCommandsCond,
												&m_mutex);
									}
									s = pthread_mutex_unlock(&m_mutex);
									if (s != 0) {
										Log( "~~~~~~~~~~MUTEX s=%d",
												s);
									}

									Log( "Executing. WAIT-<<");
								} else {
									Log( "[ERROR] Executing. ");
									SendResponseToClient(
											"7;Не могу начать выполнение команды. Внутренняя ошибка\n");
								}

								delete pDevCmd;
								Log( "Executing. WAIT 1-<<");
							}
						}
					}
					continue;
				} else {
					Log( "[SERVER] got wrong content length: %d",
							numRead);
					SendResponseToClient(
							"7;[SERVER] got wrong content length\n");
				}
				break;
			} else {
				Log( "[SERVER] got wrong packet length: %d",
						numRead);
				SendResponseToClient("7;[SERVER] got wrong packet length\n");
			}
			break;

		} while (1);
		//TODO: handle error better
		::close(m_clientSock);
		m_clientSock = -1;
	}
	Log( "DaemonCommServer::Run(): DONE");
	return NULL;
}

void DaemonCommServer::SendResponseToClient(const std::string& response) {

	int length = response.length() + 1;

	Log( "DaemonCommServer: Sending result: length: %d", length);

	unsigned char* envelope = new unsigned char[response.length() + 1
			+ sizeof(int)];

	::memcpy(envelope, &length, sizeof(int));
	if (length > 0) {
		::memcpy(envelope + sizeof(int), response.c_str(), length);
	}

	Log( "Calling send()");

	if (send(m_clientSock, envelope, length + sizeof(int), MSG_NOSIGNAL)
			== -1) {
		if (errno == EPIPE) {
			Log( "WARNING: client closed connection");
		}
	}
	delete[] envelope;
}

void DaemonCommServer::OnResultReady(DeviceCommand* pCmd) {
	Log( "DaemonCommServer::OnResultReady->>");

	SendResponseToClient(pCmd->getRawResult());

	int s = pthread_mutex_lock(&m_mutex);
	if (s != 0) {
		Log( "~~~~~~~~~~MUTEX s=%d", s);
	}

	Log( "Calling send(): locked");

	m_canProcessCommands = true;

	Log( "Calling send(): unlocked");
	pthread_cond_signal(&m_canProcessCommandsCond);
	Log( "Calling send(): after 1");

	s = pthread_mutex_unlock(&m_mutex);
	if (s != 0) {
		Log( "~~~~~~~~~~MUTEX s=%d", s);
	}

	Log( "DaemonCommServer::OnResultReady-<<");
}

void DaemonCommServer::OnDeviceStatusChanged(eDeviceStaus status) {
	Log( "OnDeviceStatusChanged: %d", status);
	if (status == DEVICE_STATUS_EXIT) {
		Cancel();
	}
}

void DaemonCommServer::OnCancel() {
	Log( "OnCancel!!!!:");

	if (m_clientSock != -1) {
		m_clientSock = -1;
		close(m_clientSock);
	}
	if (m_connectSock != -1) {
		m_connectSock = -1;
		close(m_connectSock);
	}
	pthread_cond_destroy(&m_canProcessCommandsCond);
	pthread_mutex_destroy(&m_mutex);
}

