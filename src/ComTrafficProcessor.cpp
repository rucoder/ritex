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
#include <syslog.h>

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
	: m_state(STATE_INIT), m_fd(-1), m_pDevice(pDevice), m_writeMode(WRITE_MODE_0), m_isDataCapture(false), m_pendingCmd(NULL), m_doRun(true)
{
	// TODO Auto-generated constructor stub

}

ComTrafficProcessor::~ComTrafficProcessor() {
	// TODO Auto-generated destructor stub
}

int ComTrafficProcessor::OpenCommPort(std::string port, int speed)
{
	struct termios tios;
    int fd;

	fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK| O_NDELAY );
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
    //tios.c_lflag &= ~(ICANON | ECHO | ISIG);
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

	return m_ackParams[index].m_ack;

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
		default:
			return std::string("ERROR_XXX: UNHANDLED!!!!");
	}
}

std::string ComTrafficProcessor::GetStateStr(eState state) {
	switch(state) {
	case STATE_INIT:
		return std::string("STATE_INIT");
	case STATE_GET_INIT_WRITE_MODE:
		return std::string("STATE_GET_INIT_WRITE_MODE");
	case STATE_WAIT_ACK:
		return std::string("STATE_WAIT_ACK");
	case STATE_WAIT_CMD:
		return std::string("STATE_WAIT_CMD");
	case STATE_CUSTOM_CMD:
		return std::string("STATE_CUSTOM_CMD");
	case STATE_WAIT_CUSTOM_ACK:
		return std::string("STATE_WAIT_CUSTOM_ACK");
	case STATE_SET_MODE:
		return std::string("STATE_SET_MODE");
	case STATE_WAIT_MODE_SET:
		return std::string("STATE_WAIT_MODE_SET");
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
	to.tv_sec = 0;
	to.tv_usec = timeout * 1000; //1 sec ms waiting for line activity

	syslog(LOG_ERR, "[KSU] waiting for activity");

	pPacket = ReadPacket(&to, error);

	syslog(LOG_ERR, "[Run] Activity detected: %s", GetErrorStr(error).c_str());

	return pPacket;
}



void ComTrafficProcessor::ChangeMode(unsigned short mode)
{
	SetSetting(45, mode);
}

void ComTrafficProcessor::SetSetting(unsigned char setting, unsigned short value) {
	syslog(LOG_ERR,"SetSetting(%d, %d) -->>", setting, value);
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_SETTING_SET, time(NULL));
	unsigned char* pData = pPacket->Allocate(3);
	pData[0] = setting;
	pData[1] = MSB(value);
	pData[2] = LSB(value);
	WritePacket(pPacket);
	delete pPacket;
	syslog(LOG_ERR,"SetSetting(%d, %d) --<<", setting, value);
}

void ComTrafficProcessor::GetAllSettings() {
	syslog(LOG_ERR,"GetAllSettings() -->>");
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_SETTING_ALL_GET, time(NULL));
	WritePacket(pPacket);
	delete pPacket;
	syslog(LOG_ERR,"GetAllSettings() --<<");
}


void ComTrafficProcessor::GetPasswords() {
	syslog(LOG_ERR,"GetPasswords() -->>");
	DataPacket* pPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_PASSWORD_GET, time(NULL));
	WritePacket(pPacket);
	delete pPacket;
	syslog(LOG_ERR,"GetPasswords() --<<");

}

void ComTrafficProcessor::SetPassword(unsigned short password) {
	SetSetting(44, password);
}

bool ComTrafficProcessor::SendCustomCmd(custom_command_t* cmd)
{
	pthread_mutex_lock(&m_cmdMutex);
	m_pendingCmd = cmd;
	pthread_mutex_unlock(&m_cmdMutex);
	return true;
}


//bool ComTrafficProcessor::HandleErrorForCustomCmd(eState state, int error) {
//	m_doRun = m_isDataCapture;
//}

void* ComTrafficProcessor::Run()
{
	int number_of_ksu_failures = MAX_KSU_CHANCES;

	unsigned char passwd_hi, passwd_lo;

	m_state = STATE_INIT;

	syslog(LOG_ERR, "Enter run...");

	if(!m_isDataCapture) {
		syslog(LOG_ERR, "Waiting for start...");
		pthread_mutex_lock(&m_startMutex);
			while(!m_doRun) {
				pthread_cond_wait(&m_startCond,&m_startMutex);
		pthread_mutex_unlock(&m_startMutex);
		}
	}

	syslog(LOG_ERR, "Now running...");


	while(m_doRun) {
		int error;
		DataPacket* packet = NULL;

		syslog(LOG_ERR, "#### STATE: %d %s WRITE_MODE=%d KSU CHANCES: %d", m_state, GetStateStr(m_state).c_str(), m_writeMode, number_of_ksu_failures);

		switch (m_state) {
			case STATE_GET_INIT_WRITE_MODE:
				GetAllSettings();

				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

				switch(error) {
				case ERROR_READ_NO_ERROR:
					assert(packet != NULL);

					number_of_ksu_failures = MAX_KSU_CHANCES;

					if(packet->GetType() == TYPE_CMD) {
						syslog(LOG_ERR,"ERROR!!!!!");
						m_state = STATE_INIT;
					} else {
						assert(packet->GetCmd() == ACK_ALL_SETTINGS);

						unsigned char* pData = packet->GetDataPtr();
						m_writeMode = pData[WRITE_MODE_OFFSET];
						//syslog(LOG_ERR,"<<<<<<<< CURRENT WRITE_MODE=%d", m_writeMode);
						m_state = STATE_GET_INIT_PASSWORDS;

					}
					break;
				case ERROR_READ_TIMEOUT:
					if(--number_of_ksu_failures == 0) {
						//TODO: check logic. How many times we need to report event?
						number_of_ksu_failures = MAX_KSU_CHANCES;
						//TODO: report event here
						syslog(LOG_ERR, "KSU not responding %d sec. Reporting", KSU_INACTIVITY_TIMEOUT * MAX_KSU_CHANCES);
					}
					break;
				case ERROR_READ_BAD_CRC:
					// TODO: report event
					break;
				default:
					syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
					break;
				}
				if(packet)	{
					delete packet;
				}
				break;
			case STATE_GET_INIT_PASSWORDS:
				GetPasswords();

				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

				switch(error) {
				case ERROR_READ_NO_ERROR:
					assert(packet != NULL);

					number_of_ksu_failures = MAX_KSU_CHANCES;

					if(packet->GetType() == TYPE_CMD) {
						syslog(LOG_ERR,"ERROR!!!!!");
					} else {
						assert(packet->GetCmd() == ACK_PASSWORDS);

						unsigned char* pData = packet->GetDataPtr();
						//m_writeMode = pData[20];
						syslog(LOG_ERR,"<<<<<<<< CURRENT PASSWORDS=0x%X 0x%X 0x%X 0x%X", pData[0], pData[1], pData[2], pData[3]);
						passwd_hi = pData[2];
						passwd_lo = pData[3];
						//m_state = STATE_WAIT_CMD;
						m_state = STATE_SET_CURRENT_PASSWORD;
					}
					break;
				case ERROR_READ_TIMEOUT:
					if(--number_of_ksu_failures == 0) {
						//TODO: check logic. How many times we need to report event?
						number_of_ksu_failures = MAX_KSU_CHANCES;
						//TODO: report event here
						syslog(LOG_ERR, "KSU not responding %d sec. Reporting", KSU_INACTIVITY_TIMEOUT * MAX_KSU_CHANCES);
					}
					break;
				case ERROR_READ_BAD_CRC:
					// TODO: report event
					break;
				default:
					syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
					break;
				}
				if(packet)	{
					delete packet;
				}
				break;

			case STATE_SET_CURRENT_PASSWORD:
				SetPassword((unsigned short)passwd_hi << 8 | passwd_lo);

				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
				switch(error) {
					case ERROR_READ_TIMEOUT:
						m_state = STATE_INIT;
						syslog(LOG_ERR, "KSU not responding %d sec. Reporting", KSU_INACTIVITY_TIMEOUT * MAX_KSU_CHANCES);
						//number_of_ksu_failures++;
						break;
					case ERROR_READ_NO_ERROR:
					{
						number_of_ksu_failures = MAX_KSU_CHANCES;

						//somthing bad happened. get error code
						if(packet->GetCmd() == ACK_ACK) {
							unsigned char* pData = packet->GetDataPtr();
							syslog(LOG_ERR, "!!!! FIALED SET PASSWORD %d: REASON: %d", passwd_hi << 8 | passwd_lo, pData[0]);
							m_state = STATE_INIT;
						} else {
							assert(packet->GetCmd() == ACK_ALL_SETTINGS);

							unsigned char* pData = packet->GetDataPtr();
							m_writeMode = pData[WRITE_MODE_OFFSET];
							//syslog(LOG_ERR,"<<<<<<<< CURRENT WRITE_MODE=%d", m_writeMode);
							m_state = STATE_WAIT_CMD;
						}
					}
					break;

					default:
						syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
						break;
				}
				if(packet)	{
					delete packet;
				}
				break;

			case STATE_INIT:
				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

				switch(error) {
				case ERROR_READ_NO_ERROR:
					assert(packet != NULL);

					number_of_ksu_failures = MAX_KSU_CHANCES;

					if(packet->GetType() == TYPE_CMD) {
						//wait for ACK. stay in the state
					} else {
						//we got ACK. ask for settings
						m_state = STATE_GET_INIT_WRITE_MODE;
					}
					break;
				case ERROR_READ_TIMEOUT:
					if(--number_of_ksu_failures == 0) {
						//TODO: check logic. How many times we need to report event?
						number_of_ksu_failures = MAX_KSU_CHANCES;
						//TODO: report event here
						syslog(LOG_ERR, "KSU not responding %d sec. Reporting", KSU_INACTIVITY_TIMEOUT * MAX_KSU_CHANCES);
					}
					break;
				case ERROR_READ_BAD_CRC:
					// TODO: report event
					break;
				default:
					syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
					break;
				}

				if(packet)	{
					delete packet;
				}
				break;
			/*
			 * the command was previously detected in the traffic so 'ПИУ'
			 * expects ACK within KSU_INACTIVITY_TIMEOUT so do we
			 * 1. if timeout happens 'ПИУ' will repeat the command so we need to go to INIT
			 * 2. if ACK has gotten then report data and wait for the next CMD
			 */
			case STATE_WAIT_ACK:
				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
				switch(error) {
					case ERROR_READ_TIMEOUT:
						if(--number_of_ksu_failures == 0) {
							//TODO: check logic. How many times we need to report event?
							number_of_ksu_failures = MAX_KSU_CHANCES;
							//TODO: report event here
							syslog(LOG_ERR, "KSU not responding %d sec. Reporting", KSU_INACTIVITY_TIMEOUT * MAX_KSU_CHANCES);
						}
						m_state = STATE_INIT;
						break;
					case ERROR_READ_NO_ERROR:
						//1. report data
						number_of_ksu_failures = MAX_KSU_CHANCES;

						/*
						 * the window is open to send custom command
						 * we can execute custom command in two different modes
						 * 1. 'one-shot' -- no data capture. E.g. -t, -cmd without -r before it
						 */

						if (m_isDataCapture) {
							m_pDevice->ReportDataPacket(packet);
						}

						pthread_mutex_lock(&m_cmdMutex);
						if(m_pendingCmd) {
							m_state = STATE_CUSTOM_CMD;
						} else {
							m_state = STATE_SET_MODE;
						}
						pthread_mutex_unlock(&m_cmdMutex);

						break;
					default:
						syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
						break;
				}
				if(packet)	{
					delete packet;
				}
				break;
			case STATE_CUSTOM_CMD:
			{
				syslog(LOG_ERR,"!!!!!!! Sending custom CMD 0x%X", m_pendingCmd->m_pDataPacket->GetCmd());

				if(GET_MODE(m_pendingCmd->m_pDataPacket->GetCmd()) != m_writeMode) {
					//need to change mode first
					m_writeMode = (GET_MODE(m_pendingCmd->m_pDataPacket->GetCmd()) + 6) % 7;
					m_state = STATE_SET_MODE;
					break;
				}

				WritePacket(m_pendingCmd->m_pDataPacket);

				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);

				switch(error) {
				case ERROR_READ_TIMEOUT:
					break;
				case ERROR_READ_NO_ERROR:
					syslog(LOG_ERR, "!!!!!!!! GOT REPLY FOR CUSTOM CMD: cmd=0x%X ack=0x%X", m_pendingCmd->m_pDataPacket->GetCmd(), packet->GetCmd());
					break;
				default:
					break;
				}

				//TODO: log event


				int s = pthread_mutex_lock(&m_cmdMutex);
				if( s != 0) {
					syslog(LOG_ERR, "~~~~~~~~~~MUTEX s=%d",s);
				}
				m_pendingCmd->m_pParentCommand->SetReply(packet, error);
				m_pendingCmd = NULL;
				s = pthread_mutex_unlock(&m_cmdMutex);
				if( s != 0) {
					syslog(LOG_ERR, "~~~~~~~~~~MUTEX s=%d",s);
				}


				m_state = STATE_WAIT_CMD;

				m_doRun = m_isDataCapture;
				syslog(LOG_ERR, "#### NEED RUN? m_doRun = %d", m_doRun);
				if(packet)	{
					delete packet;
				}

			}
			break;

			case STATE_SET_MODE:
				//try to set next mode. update current mode on reply only
				ChangeMode((m_writeMode+1) % 7);

				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
				switch(error) {
					case ERROR_READ_TIMEOUT:
						m_state = STATE_INIT;
						syslog(LOG_ERR, "KSU not responding %d sec. Reporting", KSU_INACTIVITY_TIMEOUT * MAX_KSU_CHANCES);
						break;
					case ERROR_READ_NO_ERROR:
					{
						number_of_ksu_failures = MAX_KSU_CHANCES;

						//something bad happened. get error code
						if(packet->GetCmd() == ACK_ACK) {
							unsigned char* pData = packet->GetDataPtr();
							syslog(LOG_ERR, "!!!! FIALED SET MODE %d: REASON: %d", (m_writeMode + 1) % 7, pData[0]);
							m_state = STATE_INIT;
						} else {
							assert(packet->GetCmd() == ACK_ALL_SETTINGS);

							unsigned char* pData = packet->GetDataPtr();
							if((m_writeMode+1) % 7 != pData[WRITE_MODE_OFFSET]) {
								syslog(LOG_ERR,"WARNING: new mode is not reflected in reply");
							}
							m_writeMode = pData[WRITE_MODE_OFFSET];
							syslog(LOG_ERR,"<<<<<<<< CURRENT WRITE_MODE=%d", m_writeMode);
							m_state = STATE_WAIT_CMD;
						}
					}
					break;

					default:
						syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
						break;
				}
				if(packet)	{
					delete packet;
				}

				break;

			case STATE_WAIT_CMD:
				packet = WaitForKsuActivity(KSU_INACTIVITY_TIMEOUT, error);
				switch(error) {
					case ERROR_READ_TIMEOUT:
						m_state = STATE_INIT;
						syslog(LOG_ERR, "KSU not responding %d sec. Reporting", KSU_INACTIVITY_TIMEOUT * MAX_KSU_CHANCES);
						break;
					case ERROR_READ_NO_ERROR:
						number_of_ksu_failures = MAX_KSU_CHANCES;
						m_state = STATE_WAIT_ACK;
						break;
					default:
						syslog(LOG_ERR, "KSU communication error: %d (%s)", error, GetErrorStr(error).c_str());
						break;
				}
				if(packet)	{
					delete packet;
				}
				break;
			default:
				break;
		}
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
#if 0
					syslog(LOG_ERR, "### ECHO FIFO : FIFO=0x%X PORT=0x%X", echoByte, ((unsigned char*)buffer)[total_read]);
#endif

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

	calculatedCrc16 = Utils::Crc16(calculatedCrc16, address);

	numRead = Read(&length, 2, timeout);
	if(numRead < 0) {
		error = numRead;
		return NULL;
	}

	calculatedCrc16 = Utils::Crc16(calculatedCrc16, (unsigned char*)&length, 2);

	length = swap16(length);

	numRead = Read(&cmd, 1, timeout);
	if(numRead < 0) {
		error = numRead;
		return NULL;
	}

	calculatedCrc16 = Utils::Crc16(calculatedCrc16, cmd);

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
			calculatedCrc16 = Utils::Crc16(calculatedCrc16, packet->GetDataPtr(), length);
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

#if 0
		//print packet data
		unsigned char* p = packet->GetDataPtr();
		for(int i = 0; i < packet->GetSize(); i++) {
			syslog(LOG_ERR, "0x%X", p[i]);
		}
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
	for(int i = 0; i < rawSize; i++) {
		m_echoCancelFifo.push(pRawPacket[i]);
	}
	write(m_fd, pRawPacket, rawSize);
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



