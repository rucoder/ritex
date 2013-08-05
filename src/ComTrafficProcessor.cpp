/*
 * ComTrafficProcessor.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: ruinmmal
 */

#include "RitexDevice.h"
#include "ComTrafficProcessor.h"
#include "DataPacket.h"
#include "Utils.h"

//comm port
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>

#include "Log.h"

//memset
#include <string.h>

//select
#include <sys/select.h>
// for EWOULDBLOCK
#include <errno.h>
//for open
#include <unistd.h>

#include <assert.h>

#define MAX_KSU_CHANCES 3
#define KSU_INACTIVITY_TIMEOUT 1500 //1 sec

#define WRITE_MODE_OFFSET 22

#define MODE_CYCLE_TIME (60) /*30*/
#define CAPTURE_TIME_LIMIT (6) // actually about 8 sec, but to be sure

#define DATA_CAPTURE_PERIOD 5

//MUST be aligned with ComTrafficProcessor::m_ackParams array
#define IDX_ACK_ACK 0
#define IDX_ACK_PASSWORDS 1
#define IDX_ACK_ALL_SETTINGS 2
#define IDX_ACK_INFO 3
#define IDX_ACK_STORED_INFO 4

#define IDX_ACK_NONE -1

ComTrafficProcessor::__tag_cmdParams ComTrafficProcessor::m_cmdParams[] = {
		{REQ_VD_OFF, 1, IDX_ACK_INFO},
		{REQ_INFO_REQUEST_MODE_0, 1, IDX_ACK_INFO},
		{REQ_DEVICE_REQ, 1, IDX_ACK_ACK},
		{REQ_SETTING_ALL_GET, 1, IDX_ACK_ALL_SETTINGS},
		{REQ_PASSWORD_GET, 1, IDX_ACK_PASSWORDS},

		{REQ_VD_ROTATION, 2, IDX_ACK_INFO},
		{REQ_VD_ON, 2, IDX_ACK_INFO},
		{REQ_SET_SPEED, 2, IDX_ACK_ACK},

		{REQ_SETTING_SET, 4, IDX_ACK_ALL_SETTINGS}, //special case. may send ACK_ACK on error
		{REQ_STORED_INFO_REQUEST, 7, IDX_ACK_STORED_INFO}
};

#define MAX_COMMAND_INDEX (sizeof(m_cmdParams) / sizeof(ComTrafficProcessor::__tag_cmdParams))

ComTrafficProcessor::__tag_cmdParams ComTrafficProcessor::m_ackParams[] = {
		{ACK_ACK, 2, IDX_ACK_NONE},
		{ACK_PASSWORDS, 5, IDX_ACK_NONE},
		{ACK_ALL_SETTINGS, 0x30, IDX_ACK_ACK}, //has an alternative reply //ERROR IN PSEC???? 0x2E

		// may have different length depends on current mode
		{ACK_INFO_MODE_0, 0x28, IDX_ACK_NONE},
		{ACK_INFO_MODE_1, 0x05, IDX_ACK_NONE},
		{ACK_INFO_MODE_2, 0x20, IDX_ACK_NONE},
		{ACK_INFO_MODE_3, 0x12, IDX_ACK_NONE},
		{ACK_INFO_MODE_4, 0x0D, IDX_ACK_NONE},
		{ACK_INFO_MODE_5, 0x12, IDX_ACK_NONE},
		{ACK_INFO_MODE_6, 0x13, IDX_ACK_NONE},


		{ACK_STORED_INFO, 0x213, IDX_ACK_NONE}
};

#define MAX_ACK_INDEX (sizeof(m_cmdParams) / sizeof(ComTrafficProcessor::__tag_cmdParams))


ComTrafficProcessor::ComTrafficProcessor(RitexDevice* pDevice)
	: m_state(STATE_PROCESS_KSU_TRAFFIC), m_nextState(STATE_PROCESS_KSU_TRAFFIC), m_fd(-1), m_pDevice(pDevice), m_writeMode(WRITE_MODE_0), m_isDataCapture(false),m_doRun(true),
	  m_isInFault(false), m_faultCode(-1), m_number_of_ksu_failures(MAX_KSU_CHANCES), m_vd_state(-1),
	  m_dataCapturePeriod(DATA_CAPTURE_PERIOD), m_motorStatus(0xFF)
{

}

ComTrafficProcessor::~ComTrafficProcessor() {

}

int ComTrafficProcessor::OpenCommPort(std::string port, int speed)
{
	struct termios tios;
    int fd;

	fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK| O_NDELAY | O_EXCL);
    if (fd < 0) {
            Log( "[COMM]: Can't open port %s\n", port.c_str());
            return -1;
    }
    tcgetattr(fd, &tios);
    //tcgetattr(fd, &(tios[TIOS_OLD])); /* save current port settings */
    //memset(&tios, 0, sizeof(tios));
    switch (speed) {
    case 1200:
    	speed = B1200;
            break;
    case 2400:
    	speed = B2400;
            break;
    case 4800:
    	speed = B4800;
            break;
    case 19200:
    	speed = B19200;
            break;
    case 38400:
    	speed = B38400;
            break;
    case 57600:
    	speed = B57600;
            break;
    case 115200:
    	speed = B115200;
            break;
    default:
    	speed = B9600;
            break;
    }
    tios.c_cflag = speed | CS8 | CLOCAL | CREAD;
    tios.c_iflag = IGNPAR;
    tios.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    tios.c_lflag = 0;

    tios.c_cc[VTIME] = 1; /* inter-character timer used */
    tios.c_cc[VMIN] = 1; /* blocking read until 1000 chars received */

    cfmakeraw(&tios);

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tios);
    return fd;

}

bool ComTrafficProcessor::isLengthMatches(unsigned char cmd, unsigned short length, int& type)
{

	//Log( "FULL CMD: 0x%X", cmd);

	for(unsigned int i = 0; i < MAX_COMMAND_INDEX; i++) {
		if(GET_CMD(m_cmdParams[i].m_cmd)  == cmd)
			if(m_cmdParams[i].m_length == length)
			{
				type = TYPE_CMD;
				return true;
			}
	}

	for(unsigned int i = 0; i < MAX_ACK_INDEX; i++) {
		if(GET_CMD(m_ackParams[i].m_cmd) == cmd)
			if(m_ackParams[i].m_length == length)
			{
				type = TYPE_ACK;
				return true;
			}
	}
	//either cmd not found or length do not match
	return false;
}

int ComTrafficProcessor::GetAckForCmd(int cmd, bool primary){
	int index = -1;
	for(unsigned int i = 0; i < MAX_COMMAND_INDEX; i++) {
		if(m_cmdParams[i].m_cmd == cmd)
			index = m_cmdParams[i].m_ack;
	}

	if (index == -1)
		return -1;

	if(!primary)
		index = m_ackParams[index].m_ack;

	if (index == -1)
		return -1;

	return m_ackParams[index].m_cmd;

}

std::string ComTrafficProcessor::GetErrorStr(int error)
{
	switch (error) {
		case ERROR_READ_BAD_CRC:
			return std::string("ERROR_READ_BAD_CRC");
		case ERROR_READ_BAD_PACKET:
			return std::string("ERROR_READ_BAD_PACKET");
		case ERROR_READ_NO_ERROR:
			return std::string("ERROR_READ_NO_ERROR");
		case ERROR_READ_OTHER:
			return std::string("ERROR_READ_OTHER");
		case ERROR_READ_TIMEOUT:
			return std::string("ERROR_READ_TIMEOUT");
		case ERROR_OOM:
			return std::string("ERROR_OOM");
		case ERROR_READ_UNEXPECTED_PACKET:
			return std::string("ERROR_READ_UNEXPECTED_PACKET");
		default:
			return std::string("ERROR_XXX: UNHANDLED!!!!");
	}
}

std::string ComTrafficProcessor::GetStateStr(eState state) {
	switch(state) {
	case STATE_PROCESS_KSU_TRAFFIC:
		return std::string("STATE_PROCESS_KSU_TRAFFIC");
	case STATE_CAPTURE_DEVICE:
		return std::string("STATE_CAPTURE_DEVICE");
	}
}

DataPacket* ComTrafficProcessor::WaitForKsuActivity(int timeout, int& error) {
	struct timeval to;
	DataPacket* pPacket;
	//reset timeout
	to.tv_sec = timeout / 1000;
	to.tv_usec = (timeout % 1000) * 1000; //waiting for line activity with timeout

#ifdef __DEBUG__
	Log( "[KSU] waiting for activity: sec:=%d usec=%lu", to.tv_sec, to.tv_usec);
#endif

	pPacket = ReadPacket(&to, error);

#ifdef __DEBUG__
	if(pPacket) {
		Log ("[TRAFFIC] CMD=0x%X", GET_CMD(pPacket->GetCmd()));
	}
#endif

#ifdef __DEBUG__
	Log( "[Run] Activity detected: %s", GetErrorStr(error).c_str());
#endif

	return pPacket;
}



bool ComTrafficProcessor::ChangeMode(unsigned short mode, int& error)
{
#ifdef __DEBUG__
	Log("ChangeMode(%d) -->>", mode);
#endif

	bool result = false;
	DataPacket* pPacket;
	unsigned char* pData;

	SetSetting(45, mode);

	pPacket = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

	if (error == ERROR_READ_NO_ERROR) {
		pData = pPacket->GetDataPtr();
		if (GET_CMD(pPacket->GetCmd()) == ACK_ALL_SETTINGS) {
			result = pData[WRITE_MODE_OFFSET] == mode;
		} else if (GET_CMD(pPacket->GetCmd()) ==  ACK_ACK ){
			Log( "[ERROR]: FIALED TO SET MODE %d: REASON: %d", (m_writeMode + 1) % 7, pData[0]);
		} else {
			error = ERROR_READ_UNEXPECTED_PACKET;
		}
	}

	if(pPacket)
		delete pPacket;

#ifdef __DEBUG__
	Log("ChangeMode() --<<");
#endif

	return result;
}

void ComTrafficProcessor::SetSetting(unsigned char setting, unsigned short value) {
#ifdef __DEBUG__
	Log("SetSetting(%d, %d) -->>", setting, value);
#endif
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_SETTING_SET, time(NULL));
	unsigned char* pData = pPacket->Allocate(3);
	pData[0] = setting;
	pData[1] = MSB(value);
	pData[2] = LSB(value);
	WritePacket(pPacket);
	delete pPacket;
#ifdef __DEBUG__
	Log("SetSetting(%d, %d) --<<", setting, value);
#endif
}

void ComTrafficProcessor::GetAllSettings() {
#ifdef __DEBUG__
	Log("GetAllSettings() -->>");
#endif
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_SETTING_ALL_GET, time(NULL));
	WritePacket(pPacket);
	delete pPacket;
#ifdef __DEBUG__
	Log("GetAllSettings() --<<");
#endif
}


bool ComTrafficProcessor::GetPasswords(int& error, unsigned char& passHi, unsigned char& passLow) {
#ifdef __DEBUG__
	Log("GetPasswords() -->>");
#endif
	bool result = false;
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_PASSWORD_GET, time(NULL));
	WritePacket(pPacket);
	delete pPacket;

	pPacket = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

	if(error == ERROR_READ_NO_ERROR) {
		if(GET_CMD(pPacket->GetCmd()) == ACK_PASSWORDS) {
			unsigned char* pData = pPacket->GetDataPtr();
			Log("CURRENT PASSWORDS=0x%X 0x%X 0x%X 0x%X", pData[0], pData[1], pData[2], pData[3]);
			passHi = pData[2];
			passLow = pData[3];
			result = true;
		} else {
			error = ERROR_READ_UNEXPECTED_PACKET;
		}
	}


#ifdef __DEBUG__
	Log("GetPasswords() --<<");
#endif

	return result;
}

bool ComTrafficProcessor::SetPassword(unsigned short password, int& error, DataPacket** settings) {
#ifdef __DEBUG__
	Log("SetPassword() -->>");
#endif
	bool result;
	DataPacket* pPacket = NULL;
	*settings = NULL;

	SetSetting(44, password);

	pPacket = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

	if (error == ERROR_READ_NO_ERROR) {
		//something bad happened. get error code
		if(pPacket->GetCmd() == ACK_ACK) {
			unsigned char* pData = pPacket->GetDataPtr();
			Log( "[ERROR] FIALED SET PASSWORD. REASON: %d", pData[0]);
			delete pPacket;
		} else if(pPacket->GetCmd() == ACK_ALL_SETTINGS) {
			*settings = pPacket;
			result =  true;
		}
	}

#ifdef __DEBUG__
	Log("SetPassword() --<<");
#endif
	return result;
}

bool ComTrafficProcessor::SendCustomCmd(custom_command_t* cmd)
{
	pthread_mutex_lock(&m_cmdMutex);
	Log("$$$$$$ CUSTOM_CMD sent!");
	Log("DataReaderThread CUSTOM_CMD m_pendingCmdQueue size: %d", m_pendingCmdQueue.size());
	m_pendingCmdQueue.push(cmd);
	Log("DataReaderThread CUSTOM_CMD m_pendingCmdQueue size: %d", m_pendingCmdQueue.size());
	pthread_mutex_unlock(&m_cmdMutex);
	return true;
}

bool ComTrafficProcessor::CheckAndReportFault(DataPacket* packet) {
	// one of INFO replies. bytes 0 and 1 has fault information
	/*
	 * FIXME: workaround for mode 1. error is set to 0 in this mode for some reason
	 */
	if(packet->GetCmd() == ACK_INFO_MODE_1)
		return false;


	if(GET_CMD(packet->GetCmd()) == ACK_INFO) {
		unsigned char vd_state =  packet->GetDataPtr()[VD_STATUS_OFFSET];
		unsigned char error_code = packet->GetDataPtr()[1];
		Log( "[ FAULT ? ]: isInFault=%d VD state=%d ERROR=%d",m_isInFault, vd_state, error_code);

		if(!m_isInFault) {
			if (error_code > 0) {
				m_faultCode = error_code;
				m_isInFault = true;
				m_pDevice->ReportFault(m_faultCode, packet->GetTimestamp());
			}
		} else {
			//clear fault status
			if (error_code == 0) {
				m_isInFault = false;
				m_faultCode = 0;

				m_pDevice->ReportFault(m_faultCode, packet->GetTimestamp());
			} else if (error_code != m_faultCode)  {//one more error detected
				m_faultCode = error_code;
				m_pDevice->ReportFault(m_faultCode, packet->GetTimestamp());
			}
		}
	}
	return false;
}

bool ComTrafficProcessor::ProcessCommand(custom_command_t* pCmd, int& error)
{
	DataPacket* pPacket = NULL;
	//send command
	WritePacket(pCmd->m_pDataPacket);

	//get reply
	pPacket = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
	do {
		if (error == ERROR_READ_NO_ERROR) {
			unsigned char cmd = GET_CMD(pCmd->m_pDataPacket->GetCmd());
			// need to ask for info and wait for reply
			if(cmd == REQ_VD_ON || cmd == REQ_VD_OFF || cmd == REQ_VD_ROTATION) {
				if(GET_CMD(pPacket->GetCmd()) == ACK_ACK) {
					if(pPacket->GetDataPtr()[0] == 0) {
						// we do not need ACK packet anymore
						delete pPacket;
						pPacket = NULL;
						// now request real command result
						RequestCurrentData();
						pPacket = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
						if(error != ERROR_READ_NO_ERROR) {
							break;
						}
					} else {
						// we know that AC_ACK matches CMD. no need to check
						break;
					}
				} else {
					error = ERROR_READ_UNEXPECTED_PACKET;
					break;
				}
			}
			// check that reply matches command
			if(!GetAckForCmd(cmd) == pPacket->GetCmd() &&
					!GetAckForCmd(pCmd->m_pDataPacket->GetCmd(), false) == pPacket->GetCmd()){
				error = ERROR_READ_UNEXPECTED_PACKET;
			}
		}
	} while(false);
	pCmd->m_pParentCommand.SetReply(pPacket, error);
	delete pCmd;
	if(pPacket)
		delete pPacket;
	//TODO: handle retry??
	return true;
}


bool ComTrafficProcessor::NeedCapture() {
	return (!m_pendingCmdQueue.empty() || (m_nextModeLoopCycle < time(NULL)));
}

/*
 * This function is called from STATE_PROCESS_KSU_TRAFFIC state
 * it must respect timing diagram of KSU
 */
void ComTrafficProcessor::ProcessMotorCmd(int cmd, int param, int& error)
{
#ifdef __DEBUG__
	Log("ProcessMotorCmd(cmd = 0x%X param = 0x%X) -->>", cmd, param);
#endif

	DataPacket* pPacket = NULL,  *pPacket2 = NULL, *pPacket3 = NULL;
	pPacket = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
	if (error == ERROR_READ_NO_ERROR) {
		//we need ACK first
		if (GET_CMD(pPacket->GetCmd()) == ACK_ACK) {
			// command accepted, wait for real status
			if (pPacket->GetDataPtr()[0] == 0) {
				pPacket2 = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
				if (error == ERROR_READ_NO_ERROR) {
					if (GET_CMD(pPacket2->GetCmd()) == REQ_INFO) {
						pPacket3 = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
						if (error == ERROR_READ_NO_ERROR) {
							if (GET_CMD(pPacket3->GetCmd()) == ACK_INFO) {
								unsigned char motorStatus = pPacket3->GetDataPtr()[0];
								Log("Got motor status: new: 0x%X old: 0x%X", motorStatus, m_motorStatus);
								if((m_motorStatus & VD_ON_MASK) != (motorStatus & VD_ON_MASK)) {
									m_pDevice->ReportStationState(motorStatus & VD_ON_MASK, m_motorStatus & VD_ON_MASK, time(NULL), std::string("HND"));
								}
								m_motorStatus = motorStatus;
							} else {
								error = ERROR_READ_UNEXPECTED_PACKET;
							}

						}
					} else {

						error = ERROR_READ_UNEXPECTED_PACKET;
					}

				}


			}

		}
	}

	if(pPacket)
		delete pPacket;

	if(pPacket2)
		delete pPacket2;

	if(pPacket3)
		delete pPacket3;

#ifdef __DEBUG__
	Log("ProcessMotorCmd() --<<");
#endif

}

void* ComTrafficProcessor::Run()
{
	m_nextState = m_state = STATE_PROCESS_KSU_TRAFFIC;


	Log( "ComTrafficProcessor: Enter run()...");

	if(!m_isDataCapture) {
		Log( "ComTrafficProcessor: Waiting for start...");
		pthread_mutex_lock(&m_startMutex);
			while(!m_doRun) {
				pthread_cond_wait(&m_startCond,&m_startMutex);
		pthread_mutex_unlock(&m_startMutex);
		}
	}

	Log( "ComTrafficProcessor: Now running...");

	// pretend we have missed the cycle so we will initialize everything
	m_nextModeLoopCycle = time(NULL);

	while(m_doRun) {
		int error;
		DataPacket* packet = NULL;

		Log( "#### NEW_STATE=%s | STATE: %d %s WRITE_MODE=%d KSU CHANCES: %d: took: %lu",GetStateStr(m_nextState).c_str(), m_state, GetStateStr(m_state).c_str(), m_writeMode, m_number_of_ksu_failures);

		m_state = m_nextState;

		switch(m_state) {
			/* capture all possible packets from KSU */
			case STATE_PROCESS_KSU_TRAFFIC:
			{
				Log("********** NEXT MOD CYCLE IN %d sec", m_nextModeLoopCycle - time(NULL));
				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT*20, error);
				if (error == ERROR_READ_NO_ERROR) {
					int cmd = GET_CMD(packet->GetCmd());
					if (cmd == ACK_INFO) {

						// get operation mode
						unsigned char* pData = packet->GetDataPtr();
						m_isAutoMode = (pData[VD_STATUS_OFFSET] & VD_MODE_AUTO_MASK) != 0;
						unsigned char newMotorStatus = pData[VD_STATUS_OFFSET];
						//station could automatically turn on/off the engine. check it

						Log("*-*-*- Checking motor status: new=0x%X old=0x%X", newMotorStatus, m_motorStatus);

						if(m_motorStatus != 0xFF) {
							if((m_motorStatus & VD_ON_MASK) != (newMotorStatus & VD_ON_MASK)) {
								m_pDevice->ReportStationState(newMotorStatus & VD_ON_MASK, m_motorStatus & VD_ON_MASK, time(NULL), std::string("HND"));
							}
						}


						m_motorStatus = newMotorStatus;

						// we got regular GAT_DATA-REPORT_DATA sequence so we are in the 'window'
						// device is not captured
						// we need to capture device in 2 cases:
						// 1. external command
						// 2. we just started so need to setup environment and get initial state
						if (NeedCapture()) {
							m_nextState = STATE_CAPTURE_DEVICE;
						} else {
							// we got data packet. report it if needed
							CheckAndReportFault(packet);
							m_pDevice->CheckAndReportTimeDiviation(packet);

							if (m_isDataCapture)
							{
								if((m_dataCapturePeriod+1) % DATA_CAPTURE_PERIOD == 0) {
									m_dataCapturePeriod = 0;
									m_pDevice->ReportDataPacket(packet);
								} else {
									m_dataCapturePeriod++;
								}
							}
						}
					// operator has changed setting by hand
					} else if (cmd == ACK_ALL_SETTINGS) {
						m_pDevice->CheckSettigsChanged(*packet);
					// operator is doing something with motor
					} else if (cmd == REQ_VD_ON || cmd == REQ_VD_OFF || cmd == REQ_VD_ROTATION) {
						int rotation = 0;
						if(cmd == REQ_VD_ROTATION) {
							rotation = packet->GetDataPtr()[0];
						}
						Log("************** MOTOR CMD *************: 0x%X 0x%X", cmd, rotation);
						ProcessMotorCmd(cmd, rotation, error);
					}
				}

				if(packet)
					delete packet;
			}
			break;

			case STATE_CAPTURE_DEVICE:
			{
				int error;
				unsigned char passHi, passLow;
				if(CaptureDevice(error)) {
					if(GetPasswords(error, passHi, passLow)) {
						DataPacket* settings = NULL;
						if(SetPassword((unsigned short)passHi << 8 | passLow, error, &settings)) {
							if(settings) {
								// got all current settings in response
								unsigned char* pData = settings->GetDataPtr();

								//current write mode set by operator
								m_writeMode = pData[WRITE_MODE_OFFSET];

								m_pDevice->CheckSettigsChanged(*settings);

								// now the device is captured and password is set
								// there are several cases
								// 1. need to process custom command
								// 2. need to cycle through modes
								custom_command_t* pCmd = NULL;
								do {
									pthread_mutex_lock(&m_cmdMutex);
									if(!m_pendingCmdQueue.empty()) {
										pCmd = m_pendingCmdQueue.front();
										m_pendingCmdQueue.pop();
									} else {
										pCmd = NULL;
									}
									pthread_mutex_unlock(&m_cmdMutex);

									if(pCmd) {
										ProcessCommand(pCmd, error);
									}
								} while(pCmd != NULL);

								if (m_nextModeLoopCycle < time(NULL)) {
									// 1. request data. Data packet will contain current state and error
									// 2. process packet
									// 3. set next mode
									// 4. wait for ack
									// 5. adjust next loop time

									for(int i = 1; i < 8; i++) {

										if(m_isAutoMode) {
											if((m_writeMode + i) %7 == 1) {
												Log("********** MOD CYCLE: SKIP MODE 1: AUTO MODE ON ");
												continue;
											}
										}
										if(!ChangeMode((m_writeMode+i) % 7, error))
											break;
										Log("############ KSU mode:%d.",(m_writeMode + i) %7);
										RequestCurrentData();
										packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

										packet->SetCmdMode((m_writeMode+i) % 7);/* set mode */
										if(error == ERROR_READ_NO_ERROR) {
											m_pDevice->ReportDataPacket(packet);
										}
										if(packet)
											delete packet;



									}
									m_nextModeLoopCycle = time(NULL) + MODE_CYCLE_TIME + 5; // 5 is a mesured timeoutto exit capture mode
								}

								// go back to main processing loop
								m_nextState = STATE_PROCESS_KSU_TRAFFIC;
								//TODO: adjust timeout
							}
						}
					}
				}
				//TODO: handle error
			}
			break;
		}
		m_doRun = m_isDataCapture || !m_pendingCmdQueue.empty();
	}

	pthread_mutex_lock(&m_cmdMutex);
	while(!m_pendingCmdQueue.empty()) {
		delete m_pendingCmdQueue.front();
		m_pendingCmdQueue.pop();
	}
	pthread_mutex_unlock(&m_cmdMutex);

	m_pDevice->NotifyStatusChanged(DEVICE_STATUS_EXIT);
	Log( "Exit run...");
	return NULL;
}

/*
 * do not return until all required bytes read or timeout
 */
int ComTrafficProcessor::Read(void* buffer, int length, struct timeval* timeout)
{
	fd_set readfds, errorfds;
	int ret;
	int total_read = 0;

	FD_ZERO(&readfds);
	FD_SET(m_fd, &readfds);
	FD_ZERO(&errorfds);
	FD_SET(m_fd, &errorfds);

	do {
		ret = read(m_fd, (unsigned char*)buffer + total_read, length - total_read);


		if(ret < 0) {
			if(errno == EWOULDBLOCK) {
				//Log("WOULDBLOCK");
				// wait for event
				ret = select(m_fd + 1, &readfds, NULL, &errorfds, timeout);

				//timeout
				if(ret == 0) {
					return ERROR_READ_TIMEOUT;
				} else if (ret == -1) { //some fatal error
					return ERROR_READ_OTHER;
				} else { //got descriptor number
					if(FD_ISSET(m_fd, &errorfds)) { //error on descriptor
						return ERROR_READ_OTHER;
					}
					if(FD_ISSET(m_fd, &readfds)) { //got data. just reread
						continue;
					}
				}

			} else {
				return ERROR_READ_OTHER;
			}
		} else {
			//we got at lease ret bytes here. check if we have them in fifo. if yes, do not ++ total read
			total_read += ret;
			for(int i = 0; i < ret; i++) {
				if(m_echoCancelFifo.empty())
					break; //true bytes, not echo
				else
				{
					//roll back buffer pointer
					unsigned char echoByte = m_echoCancelFifo.front();
					m_echoCancelFifo.pop();
					total_read--;
#ifdef __DEBUG__
					Log( "### ECHO FIFO : FIFO=0x%X PORT=0x%X", echoByte, ((unsigned char*)buffer)[total_read]);
#endif

					if(echoByte != ((unsigned char*)buffer)[total_read]) {

						Log( "### [WARNING] ECHO FIFO : total_read=%d ret=%d size: %d FIFO=0x%X PORT=0x%X", total_read, ret, m_echoCancelFifo.size(), echoByte, ((unsigned char*)buffer)[total_read]);
						/*
						 * try to clear FIFO and data on port. return error so we will retry to reset to INIT state
						 */
						while(!m_echoCancelFifo.empty())
							m_echoCancelFifo.pop();

						tcflush(m_fd, TCIOFLUSH);
						return ERROR_READ_OTHER;
					}

				}
			}
		}
	} while(length != total_read);
	return total_read;
}

DataPacket* ComTrafficProcessor::ReadPacket(struct timeval* timeout, int &error)
{
	int numRead;
	unsigned char address = 0;
	unsigned short length;
	unsigned char cmd;
	unsigned short longCmd;
	unsigned short crc16 = 0, calculatedCrc16 = 0;
	int type;

	while(address != KSU_ADDRESS) {
		numRead = Read(&address, 1, timeout);
		if(numRead < 0) {
			error = numRead;
			return NULL;
		}
		//we got activity on the line so decrease timeout
		timeout->tv_sec = 0;
		//TODO: IMPORTANT: get the right value
		timeout->tv_usec = 100 * 1000;
	}

	calculatedCrc16 = Crc16(calculatedCrc16, address);

	numRead = Read(&length, 2, timeout);
	if(numRead < 0) {
		error = numRead;
		return NULL;
	}

	calculatedCrc16 = Crc16(calculatedCrc16, (unsigned char*)&length, 2);

	length = swap16(length);

	numRead = Read(&cmd, 1, timeout);
	if(numRead < 0) {
		error = numRead;
		return NULL;
	}

	calculatedCrc16 = Crc16(calculatedCrc16, cmd);

	//Log( "GOT: CMD=0x%X LEN=%d", cmd, length);

	longCmd = cmd;
	//
	if(cmd == ACK_INFO_MODE_0) {
		longCmd = m_writeMode << 8 | cmd;
	}

	//probably good packet
	if (isLengthMatches(longCmd, length, type)) {
		//command is included in the length
		length--;

		DataPacket* packet = new DataPacket(type, address, longCmd, time(NULL));

		if(packet == NULL) {
			error = ERROR_OOM;
			return NULL;
		}

		// read the rest if needed and check CRC
		if(length > 0) {
			unsigned char * p = packet->Allocate(length);
			if(p == NULL){
				error = ERROR_OOM;
				return NULL;
			}
			numRead = Read((void*)p, length,timeout);

			if(numRead < 0) {
				delete packet;
				error = numRead;
				return NULL;
			}
			calculatedCrc16 = Crc16(calculatedCrc16, packet->GetDataPtr(), length);
		}

		// read CRC16
		numRead = Read(&crc16, 2, timeout);

		if(numRead < 0) {
			delete packet;
			error = numRead;
			return NULL;
		}

		crc16 = swap16(crc16);

		if (crc16 != calculatedCrc16) {
			Log( "CRC ERROR CRC16=0x%X CALCULATED=0x%X", crc16, calculatedCrc16);
			delete packet;
			error = ERROR_READ_BAD_CRC;
			return NULL;
		}

		Log( "Packet: addr: 0x%X cmd 0x%X length 0x%X", packet->GetAddress(), packet->GetCmd(), packet->GetSize());

#if defined(__DEBUG__) && defined (__DUMP_RAW_DATA)
		//print packet data
		unsigned char* p = packet->GetDataPtr();
		for(int i = 0; i < packet->GetSize(); i++) {
			Log( "0x%X", p[i]);
		}
		Log( "CRC: 0x%X", crc16);
#endif

		error = ERROR_READ_NO_ERROR;
		return packet;
	}
	error = ERROR_READ_BAD_PACKET;
	return NULL;
}


bool ComTrafficProcessor::WritePacket(DataPacket* pPacket) {
	int rawSize;
	unsigned char* pRawPacket = pPacket->CreateRawPacket(rawSize);
	//put bytes in echo cancellation FIFO
#if !defined(KSU_EMULATOR) && !defined(TTY_NO_ECHO)
	for(int i = 0; i < rawSize; i++) {
		m_echoCancelFifo.push(pRawPacket[i]);
	}
#endif

	struct timespec tm;

	/*
	 * this is an ugly workaround since we do not have CTS
	 * signal on the port. just wait for 100 ms.
	 */
	tm.tv_sec = 0;
	tm.tv_nsec = 100 * 1000000;

	nanosleep(&tm,NULL);

	int ret = write(m_fd, pRawPacket, rawSize);

	if(ret != rawSize) {
		Log( "[[ ERROR ]]: WritePacket: ret=%d rawSize=%d", ret, rawSize);
	}

	delete [] pRawPacket;
	return true;
}

bool ComTrafficProcessor::CaptureDevice(int& error)
{
#ifdef __DEBUG__
	Log("CaptureDevice()-->>");
#endif

	bool result = false;

	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_DEVICE_REQ, time(NULL));
	WritePacket(pPacket);
	delete pPacket;

	/* now we have to wait for ack*/
	pPacket = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

	if(error == ERROR_READ_NO_ERROR) {
		if(GET_CMD(pPacket->GetCmd()) == ACK_ACK) {
			result = (pPacket->GetDataPtr()[0] == 150);
		}
	}

	if(pPacket)
		delete pPacket;

#ifdef __DEBUG__
	Log("CaptureDevice() --<<");
#endif

	return result;
}

void ComTrafficProcessor::RequestCurrentData()
{
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_INFO, time(NULL));
	WritePacket(pPacket);
}



bool ComTrafficProcessor::Create(std::string port, int speed, bool oneShot)
{
	if(!isRunning()) {
		m_fd = OpenCommPort(port, speed);
		if (m_fd == -1) {
			return false;
		}
		pthread_mutex_init(&m_cmdMutex, NULL);

		pthread_mutex_init(&m_startMutex, NULL);
	    pthread_cond_init(&m_startCond, NULL);
		m_isDataCapture = !oneShot;
		m_doRun = !oneShot;
		return Thread::Create();
	}
	return true;
}



