/*
 * ComTrafficProcessor.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: ruinmmal
 */

#include "ComTrafficProcessor.h"

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

#include <unistd.h>

#include <errno.h>

//need to swap byte if running on LE target (e.g. x86)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline unsigned short swap16(unsigned short x)
{
    return ( (x << 8) | (x >> 8) );
}
#else
#define swap16(x) (x)
#endif

#define ERROR_READ_NO_ERROR 0
#define ERROR_READ_BAD_CRC  -1
#define ERROR_READ_TIMEOUT  -2
#define ERROR_READ_BAD_PACKET  -3
#define ERROR_READ_OTHER     -4


//MUST be aligned with ComTrafficProcessor::m_ackParams array
#define IDX_ACK_ACK 0
#define IDX_ACK_PASSWORDS 1
#define IDX_ACK_ALL_SETTINGS 2
#define IDX_ACK_INFO 3
#define IDX_ACK_STORED_INFO 4

ComTrafficProcessor::__tag_cmdParams ComTrafficProcessor::m_cmdParams[] = {
		{REQ_VD_OFF, 1, IDX_ACK_ACK},
		{REQ_INFO_REQUEST, 1, IDX_ACK_INFO},
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
		{ACK_ACK, 2, 0},
		{ACK_PASSWORDS, 5, 0},
		{ACK_ALL_SETTINGS, 0x2E, IDX_ACK_ACK}, //has an alternative reply
		{ACK_INFO, 0x28, 0},
		{ACK_STORED_INFO, 0x213, 0}
};

#define MAX_ACK_INDEX (sizeof(m_cmdParams) / sizeof(ComTrafficProcessor::__tag_cmdParams))


ComTrafficProcessor::ComTrafficProcessor()
	: m_state(STATE_INIT), m_fd(-1)
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

	fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd < 0) {
            syslog(LOG_ERR, "[COMM]: Can't open port %s\n", port.c_str());
            return -1;
    }
    //tcgetattr(fd, &(tios[TIOS_OLD])); /* save current port settings */
    memset(&tios, 0, sizeof(tios));
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
    tios.c_cc[VMIN] = 200; /* blocking read until 1000 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tios);
    return fd;

}

bool ComTrafficProcessor::isLengthMatches(unsigned char cmd, unsigned short length, int& type)
{
	for(unsigned int i = 0; i < MAX_COMMAND_INDEX; i++) {
		if(m_cmdParams[i].m_cmd == cmd)
			if(m_cmdParams[i].m_length == length)
			{
				type = TYPE_CMD;
				return true;
			}
	}

	for(unsigned int i = 0; i < MAX_ACK_INDEX; i++) {
		if(m_ackParams[i].m_cmd == cmd)
			if(m_ackParams[i].m_length == length)
			{
				type = TYPE_ACK;
				return true;
			}
	}
	//either cmd not found or length do not match
	return false;
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
		default:
			return std::string("ERROR_XXX: UNHANDLED!!!!");
	}
}

void* ComTrafficProcessor::Run()
{

	int type;

	struct timeval timeout;

	syslog(LOG_ERR, "Enter run...");
	for(;;) {
		int error;

		//reset timeout
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000 * 1000; //1 sec ms waiting for line activity


		syslog(LOG_ERR, "[Run] waiting for packet");

		struct __tag_packet* packet = ReadPacket(&timeout, error);

		if (packet == NULL) {
			syslog(LOG_ERR, "[Run] error getting packet: %s", GetErrorStr(error).c_str());
		} else { //got packet. process it
			syslog(LOG_ERR, "Got packet: address=0x%X cmd=0x%X length=%d type=%s", packet->address, packet->cmd, packet->length, packet->type == TYPE_CMD ? "CMD" : "ACK");
			delete packet;
		}
	}
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
		ret = read(m_fd, buffer, length);

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
			total_read += ret;
		}
	} while(length != total_read);
	return total_read;
}

struct ComTrafficProcessor::__tag_packet* ComTrafficProcessor::ReadPacket(struct timeval* timeout, int &error)
{
	int numRead;
	unsigned char address = 0;
	unsigned short length;
	unsigned char cmd;
	unsigned short crc16 = 0, calculatedCrc16 = 0;
	int type;

	while(address != 0x36) {
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

	//probably good packet
	if (isLengthMatches(cmd, length, type)) {
		//command is included in the length
		length--;

		struct __tag_packet* packet = (struct __tag_packet*)new unsigned char[length + 70 /* including length and command */];

		// read the rest if needed and check CRC
		if(length > 0) {
			numRead = Read((void*)&(packet->data[0]), length,timeout);

			if(numRead < 0) {
				delete packet;
				error = numRead;
				return NULL;
			}
			calculatedCrc16 = Crc16(calculatedCrc16, packet->data, length);
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

		packet->address = address;
		packet->cmd = cmd;
		packet->length = length;
		packet->type = type;
		error = ERROR_READ_NO_ERROR;
		return packet;
	}
	error = ERROR_READ_BAD_PACKET;
	return NULL;
}

unsigned short ComTrafficProcessor::Crc16(unsigned short crcInit, unsigned char byte) {
	  unsigned short lb = byte;
	  int i;
	  crcInit ^= lb << 8;
	  for (i=0; i < 8; i++)
	  {
		  if ( (crcInit & (1 << 15)) != 0)
			  crcInit = (crcInit << 1) ^ 0x8005;
		  else
			  crcInit  = crcInit << 1;
	  }
	  return  crcInit;
}


unsigned short ComTrafficProcessor::Crc16(unsigned short crcInit, unsigned char buffer[], int size) {
	for (int i = 0; i < size; i++)
		crcInit = Crc16(crcInit, buffer[i]);
	return crcInit;
}

bool ComTrafficProcessor::Create(std::string port, int speed)
{
	if(!isRunning()) {
		m_fd = OpenCommPort(port, speed);
		if (m_fd == -1) {
			return false;
		}
		return Thread::Create();
	}
	return true;
}



