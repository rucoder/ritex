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

//syslog
#ifdef KSU_EMULATOR
#define syslog(x, y, ...) printf(#y"\n", ## __VA_ARGS__)
#else
#include <syslog.h>
#endif

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




//MUST be aligned with ComTrafficProcessor::m_ackParams array
#define IDX_ACK_ACK 0
#define IDX_ACK_PASSWORDS 1
#define IDX_ACK_ALL_SETTINGS 2
#define IDX_ACK_INFO 3
#define IDX_ACK_STORED_INFO 4

#define IDX_ACK_NONE -1

ComTrafficProcessor::__tag_cmdParams ComTrafficProcessor::m_cmdParams[] = {
		{REQ_VD_OFF, 1, IDX_ACK_ACK},
		{REQ_INFO_REQUEST_MODE_0, 1, IDX_ACK_INFO},
		{REQ_DEVICE_REQ, 1, IDX_ACK_ACK},
		{REQ_SETTING_ALL_GET, 1, IDX_ACK_ALL_SETTINGS},
		{REQ_PASSWORD_GET, 1, IDX_ACK_PASSWORDS},

		{REQ_VD_ROTATION, 2, IDX_ACK_ACK},
		{REQ_VD_ON, 2, IDX_ACK_ACK},
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
	: m_state(STATE_INIT), m_nextState(STATE_INIT), m_fd(-1), m_pDevice(pDevice), m_writeMode(WRITE_MODE_0), m_isDataCapture(false), m_pendingCmd(NULL), m_doRun(true),
	  m_isInFault(false), m_faultCode(-1), m_currentSettings(NULL), m_number_of_ksu_failures(MAX_KSU_CHANCES)
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
            syslog(LOG_ERR, "[COMM]: Can't open port %s\n", port.c_str());
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

	//syslog(LOG_ERR, "FULL CMD: 0x%X", cmd);

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
	case STATE_INIT:
		return std::string("STATE_INIT");
	case STATE_WAIT_ACK:
		return std::string("STATE_WAIT_ACK");
	case STATE_WAIT_CMD:
		return std::string("STATE_WAIT_CMD");
	case STATE_CUSTOM_CMD:
		return std::string("STATE_CUSTOM_CMD");
	case STATE_SET_MODE:
		return std::string("STATE_SET_MODE");
	case STATE_GET_INIT_PASSWORDS:
		return std::string("STATE_GET_INIT_PASSWORDS");
	case STATE_SET_CURRENT_PASSWORD:
		return std::string("STATE_SET_CURRENT_PASSWORD");
	}
}

DataPacket* ComTrafficProcessor::WaitForKsuActivity(int timeout, int& error) {
	struct timeval to;
	DataPacket* pPacket;
	//reset timeout
	to.tv_sec = timeout / 1000;
	to.tv_usec = (timeout % 1000) * 1000; //waiting for line activity with timeout

#ifdef __DEBUG__
	syslog(LOG_ERR, "[KSU] waiting for activity: sec:=%d usec=%lu", to.tv_sec, to.tv_usec);
#endif

	pPacket = ReadPacket(&to, error);

#ifdef __DEBUG__
	syslog(LOG_ERR, "[Run] Activity detected: %s", GetErrorStr(error).c_str());
#endif

	return pPacket;
}



void ComTrafficProcessor::ChangeMode(unsigned short mode)
{
	SetSetting(45, mode);
}

void ComTrafficProcessor::SetSetting(unsigned char setting, unsigned short value) {
#ifdef __DEBUG__
	syslog(LOG_ERR,"SetSetting(%d, %d) -->>", setting, value);
#endif
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_SETTING_SET, time(NULL));
	unsigned char* pData = pPacket->Allocate(3);
	pData[0] = setting;
	pData[1] = MSB(value);
	pData[2] = LSB(value);
	WritePacket(pPacket);
	delete pPacket;
#ifdef __DEBUG__
	syslog(LOG_ERR,"SetSetting(%d, %d) --<<", setting, value);
#endif
}

void ComTrafficProcessor::GetAllSettings() {
#ifdef __DEBUG__
	syslog(LOG_ERR,"GetAllSettings() -->>");
#endif
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_SETTING_ALL_GET, time(NULL));
	WritePacket(pPacket);
	delete pPacket;
#ifdef __DEBUG__
	syslog(LOG_ERR,"GetAllSettings() --<<");
#endif
}


void ComTrafficProcessor::GetPasswords() {
#ifdef __DEBUG__
	syslog(LOG_ERR,"GetPasswords() -->>");
#endif
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_PASSWORD_GET, time(NULL));
	WritePacket(pPacket);
	delete pPacket;
#ifdef __DEBUG__
	syslog(LOG_ERR,"GetPasswords() --<<");
#endif

}

void ComTrafficProcessor::SetPassword(unsigned short password) {
	SetSetting(44, password);
}

bool ComTrafficProcessor::SendCustomCmd(custom_command_t* cmd)
{
	pthread_mutex_lock(&m_cmdMutex);
	syslog(LOG_ERR,"$$$$$$ CUSTOM_CMD sent!");
	m_pendingCmd = cmd;
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
		syslog(LOG_ERR, "[ FAULT ? ]: isInFault=%d VD state=%d ERROR=%d",m_isInFault, vd_state, error_code);
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

bool ComTrafficProcessor::HandleError(int error) {
#ifdef __DEBUG__
	syslog(LOG_ERR, "HandleError ->>");
#endif
	switch(error) {
	case ERROR_READ_NO_ERROR:
		m_number_of_ksu_failures = MAX_KSU_CHANCES;
		break;
	case ERROR_OOM:
	case ERROR_READ_BAD_CRC:
	case ERROR_READ_BAD_PACKET:
	case ERROR_READ_OTHER:
	case ERROR_READ_UNEXPECTED_PACKET:
	case ERROR_READ_TIMEOUT:
		//we failed to do what we want for MAX_KSU_CHANSES times is a row.
		// now either end processing or reset to INIT state
		if(--m_number_of_ksu_failures == 0) {
			m_number_of_ksu_failures = MAX_KSU_CHANCES;
			syslog(LOG_ERR, "KSU failed %d times in a row", MAX_KSU_CHANCES);
			pthread_mutex_lock(&m_cmdMutex);
			if(m_pendingCmd /*&& m_state == STATE_CUSTOM_CMD*/) {
				m_pendingCmd->m_pParentCommand.SetReply(NULL, error);
				delete m_pendingCmd;
				m_pendingCmd = NULL;
			}
			pthread_mutex_unlock(&m_cmdMutex);

			m_nextState = STATE_INIT;
			break;
		}
		syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
		break;
	}
	//either in capture mode or custom command is being executed
	m_doRun = m_isDataCapture || (m_pendingCmd != NULL);

#ifdef __DEBUG__
	syslog(LOG_ERR, "HandleError -<<");
#endif

	return true;
}

void* ComTrafficProcessor::Run()
{
	m_nextState = m_state = STATE_INIT;
	unsigned char passwd_hi, passwd_lo;


	syslog(LOG_ERR, "ComTrafficProcessor: Enter run()...");

	if(!m_isDataCapture) {
		syslog(LOG_ERR, "ComTrafficProcessor: Waiting for start...");
		pthread_mutex_lock(&m_startMutex);
			while(!m_doRun) {
				pthread_cond_wait(&m_startCond,&m_startMutex);
		pthread_mutex_unlock(&m_startMutex);
		}
	}

	syslog(LOG_ERR, "ComTrafficProcessor: Now running...");

	unsigned long delta;
	struct timeval state_run_start;
	struct timeval state_run_end;

	while (m_doRun) {
		int error;
		DataPacket* packet = NULL;



		state_run_end.tv_sec-=state_run_start.tv_sec;
		state_run_end.tv_usec-=state_run_start.tv_usec;

		delta = state_run_end.tv_sec*1000 + state_run_end.tv_usec/1000;

		gettimeofday(&state_run_start, NULL);

		syslog(LOG_ERR, "#### NEW_STATE=%s | STATE: %d %s WRITE_MODE=%d KSU CHANCES: %d: took: %lu",GetStateStr(m_nextState).c_str(), m_state, GetStateStr(m_state).c_str(), m_writeMode, m_number_of_ksu_failures, delta);

		m_state = m_nextState;

		packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

		switch(m_state) {

			case STATE_INIT:
			{
				if(error == ERROR_READ_NO_ERROR) {
					// wait for 0x64
					if(GET_CMD(packet->GetCmd()) == ACK_INFO) {
						GetPasswords();
						m_nextState = STATE_GET_INIT_PASSWORDS;
					}
				}
				HandleError(error);
				if(packet)
					delete packet;
				gettimeofday(&state_run_end, NULL);
			}
			break;

			case STATE_GET_INIT_PASSWORDS:
			{
				if(error == ERROR_READ_NO_ERROR) {
					if(packet->GetCmd() == ACK_PASSWORDS) {
						unsigned char* pData = packet->GetDataPtr();
						syslog(LOG_ERR,"CURRENT PASSWORDS=0x%X 0x%X 0x%X 0x%X", pData[0], pData[1], pData[2], pData[3]);
						passwd_hi = pData[2];
						passwd_lo = pData[3];
						SetPassword((unsigned short)passwd_hi << 8 | passwd_lo);
						m_nextState = STATE_SET_CURRENT_PASSWORD;
					} else {
						error = ERROR_READ_UNEXPECTED_PACKET;
					}
				}
				HandleError(error);
				if(packet)
					delete packet;
				gettimeofday(&state_run_end, NULL);

			}
			break;

			case STATE_SET_CURRENT_PASSWORD:
			{

				if(error == ERROR_READ_NO_ERROR) {
					//something bad happened. get error code
					if(packet->GetCmd() == ACK_ACK) {
						unsigned char* pData = packet->GetDataPtr();
						syslog(LOG_ERR, "[ERROR] FIALED SET PASSWORD. REASON: %d", pData[0]);
						m_nextState = STATE_INIT;
						//FIXME?????
					} else if(packet->GetCmd() == ACK_ALL_SETTINGS) {
						// got all current settings in response
						unsigned char* pData = packet->GetDataPtr();
						m_writeMode = pData[WRITE_MODE_OFFSET];
						if(m_currentSettings) {
							m_pDevice->CheckSettigsChanged(*packet, *m_currentSettings);
							delete m_currentSettings;
						}
						m_currentSettings = packet;
						m_nextState = STATE_WAIT_CMD;
						/*
						 * at this point we have all we need:
						 * 1. current settings array
						 * 2. current write mode
						 * 3. password successfully set
						 * we can go not to the main loop
						 */
						gettimeofday(&state_run_end, NULL);

						break; //do brake so the packet will not be deleted
					} else {
						error = ERROR_READ_UNEXPECTED_PACKET;
						//retry
						SetPassword((unsigned short)passwd_hi << 8 | passwd_lo);
					}
				}
				HandleError(error);
				if(packet)
					delete packet;
				gettimeofday(&state_run_end, NULL);

			}
			break;

			case STATE_WAIT_CMD:
			{
				if (error == ERROR_READ_NO_ERROR) {
					if(GET_CMD(packet->GetCmd()) == REQ_INFO) {
						m_nextState = STATE_WAIT_ACK;
					} else {
						error = ERROR_READ_UNEXPECTED_PACKET;
					}
				}
				HandleError(error);
				if(packet)
					delete packet;
				gettimeofday(&state_run_end, NULL);

			}
			break;
			case STATE_WAIT_ACK:
			{
				if (error == ERROR_READ_NO_ERROR)
				{
					if(GET_CMD(packet->GetCmd()) == ACK_INFO)
					{
						CheckAndReportFault(packet);
						m_pDevice->CheckAndReportTimeDiviation(packet);

						if (m_isDataCapture)
						{
							m_pDevice->ReportDataPacket(packet);
						}

						pthread_mutex_lock(&m_cmdMutex);
						if(m_pendingCmd)
						{
							//check if we need to change mode first
							if(GET_MODE(m_pendingCmd->m_pDataPacket->GetCmd()) != m_writeMode) {
								ChangeMode(GET_MODE(m_pendingCmd->m_pDataPacket->GetCmd()));
								m_nextState = STATE_SET_MODE;
							} else {
								WritePacket(m_pendingCmd->m_pDataPacket);
								m_nextState = STATE_CUSTOM_CMD;
							}
						} else {
							/*
							 * we cannot change mode if the engine is ON
							 */
#if 0
							unsigned char* pData = packet->GetDataPtr();

							if((pData[VD_STATUS_OFFSET] & VD_ON_MASK) ||
									(pData[VD_STATUS_OFFSET] & VD_MODE_AUTO_MASK)) {
								m_nextState = STATE_WAIT_CMD;
								//FIXME: is this true?
								//m_writeMode = 0; //the mode is reset to 0
							} else {
#endif
								ChangeMode((m_writeMode+1) % 7);
								m_nextState = STATE_SET_MODE;
#if 0
							}
#endif
						}
						pthread_mutex_unlock(&m_cmdMutex);

					}
				    else
					{
						error = ERROR_READ_UNEXPECTED_PACKET;
					}
				}
				HandleError(error);
				if(packet)
					delete packet;
				gettimeofday(&state_run_end, NULL);

			}
			break;
			case STATE_SET_MODE:
			{
				if(error == ERROR_READ_NO_ERROR) {

					unsigned char* pData = packet->GetDataPtr();

					if(GET_CMD(packet->GetCmd()) == ACK_ALL_SETTINGS) {
						//get new mode
						if((m_writeMode+1) % 7 != pData[WRITE_MODE_OFFSET]) {
							syslog(LOG_ERR,"[WARNING]: new mode is not reflected in reply");
						}
						m_writeMode = pData[WRITE_MODE_OFFSET];
						syslog(LOG_ERR,"CURRENT WRITE_MODE=%d", m_writeMode);
						m_nextState = STATE_WAIT_CMD;

						if(m_currentSettings) {
							m_pDevice->CheckSettigsChanged(*packet, *m_currentSettings);
							delete m_currentSettings;
						}
						m_currentSettings = packet;
						gettimeofday(&state_run_end, NULL);

						break; //keep packet
					//couldn't set mode
					} else if (GET_CMD(packet->GetCmd()) ==  ACK_ACK ){
						syslog(LOG_ERR, "[ERROR]: FIALED TO SET MODE %d: REASON: %d", (m_writeMode + 1) % 7, pData[0]);
						m_nextState = STATE_INIT;
						//FIXME: ????? to INIT???
					} else {
						error = ERROR_READ_UNEXPECTED_PACKET;
						//resend
						ChangeMode((m_writeMode+1) % 7);
					}
				}
				HandleError(error);
				if(packet)
					delete packet;
				gettimeofday(&state_run_end, NULL);

			}
			break;

			case STATE_CUSTOM_CMD:
			{
				if (error == ERROR_READ_NO_ERROR) {
					if(GetAckForCmd(m_pendingCmd->m_pDataPacket->GetCmd()) == packet->GetCmd() ||
							GetAckForCmd(m_pendingCmd->m_pDataPacket->GetCmd(), false) == packet->GetCmd()){

//						if(packet->GetCmd() == ACK_ALL_SETTINGS) {
//							if(m_currentSettings) {
//								m_pDevice->CheckSettigsChanged(*packet, *m_currentSettings, m_pendingCmd->m_pParentCommand.GetParentCmd());
//								delete m_currentSettings;
//							}
//							m_currentSettings = packet;
//						}

						// report result
						m_pendingCmd->m_pParentCommand.SetReply(packet, error);

						delete m_pendingCmd;
						m_pendingCmd = NULL;

						m_nextState = STATE_WAIT_CMD;

						gettimeofday(&state_run_end, NULL);

						break;
					} else {
						error = ERROR_READ_UNEXPECTED_PACKET;
						//retry
						WritePacket(m_pendingCmd->m_pDataPacket);
					}
				}
				HandleError(error);
				if(packet)
					delete packet;

				gettimeofday(&state_run_end, NULL);
			}
			break;
		}
#ifdef __DEBUG__
		syslog(LOG_ERR, "DORUN======%d", m_doRun);
#endif
	}

	//clean up
	if(m_currentSettings) {
		delete m_currentSettings;
		m_currentSettings = NULL;
	}

	if(m_pendingCmd) {
		delete m_pendingCmd;
		m_pendingCmd = NULL;
	}

	m_pDevice->NotifyStatusChanged(DEVICE_STATUS_EXIT);
	syslog(LOG_ERR, "Exit run...");
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
				//syslog(LOG_ERR,"WOULDBLOCK");
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
					syslog(LOG_ERR, "### ECHO FIFO : FIFO=0x%X PORT=0x%X", echoByte, ((unsigned char*)buffer)[total_read]);
#endif

					if(echoByte != ((unsigned char*)buffer)[total_read]) {

						syslog(LOG_ERR, "### ECHO FIFO : total_read=%d ret=%d size: %d FIFO=0x%X PORT=0x%X", total_read, ret, m_echoCancelFifo.size(), echoByte, ((unsigned char*)buffer)[total_read]);
					}

					assert(echoByte == ((unsigned char*)buffer)[total_read]);
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

	//syslog(LOG_ERR, "GOT: CMD=0x%X LEN=%d", cmd, length);

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
			syslog(LOG_ERR, "CRC ERROR CRC16=0x%X CALCULATED=0x%X", crc16, calculatedCrc16);
			delete packet;
			error = ERROR_READ_BAD_CRC;
			return NULL;
		}

		syslog(LOG_ERR, "Packet: addr: 0x%X cmd 0x%X length 0x%X", packet->GetAddress(), packet->GetCmd(), packet->GetSize());

#ifdef __DEBUG__
		//print packet data
		unsigned char* p = packet->GetDataPtr();
		for(int i = 0; i < packet->GetSize(); i++) {
			syslog(LOG_ERR, "0x%X", p[i]);
		}
		syslog(LOG_ERR, "CRC: 0x%X", crc16);
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
		syslog(LOG_ERR, "[[ ERROR ]]: WritePacket: ret=%d rawSize=%d", ret, rawSize);
	}

	delete [] pRawPacket;
	return true;
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



