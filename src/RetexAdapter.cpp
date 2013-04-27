/*
 * RitexAdapter.cpp
 *
 *  Created on: Apr 20, 2013
 *      Author: ruinmmal
 */

#include "RetexAdapter.h"
#include "RitexDevice.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>

//pthread
#include <pthread.h>

char* RitexAdapter::m_commDevices[] {
	"/dev/ttySC0",
	"/dev/ttySC1",
	"/dev/ttySC2",
	"/dev/ttySC3",
	"/dev/ttySC4"
};


char* RitexAdapter::m_commSpeed[] {
	"75", "110", "300","1200","2400","4800","9600","19200","38400","57600","115200"
};



RitexAdapter::RitexAdapter(std::string name, std::string version, std::string description,CmdLineParser* parser)
	:Adapter(name, version, description, parser)
{

	//populate device tree structure
	m_pDevice = new RitexDevice(this);
	AddAdditionalParameterFloat("test1", 0.4, 8.55555789);
	AddAdditionalParameter("comdevice", m_commDevices, sizeof(m_commDevices)/ sizeof(char*), LIST_VALUE_STRING);
	AddAdditionalParameter("baudrate", m_commSpeed, sizeof(m_commSpeed)/ sizeof(char*), LIST_VALUE_INT);
	AddAdditionalParameter("debug",0,1);



	AddAdditionalParameter("address","54");
}

bool RitexAdapter::isDaemonRunning() {
	/* if PID file exists and we cannot obtain lock then daemon is running*/
	int fd = open(m_pidFileName.c_str(), O_RDWR);
	if (fd > 0) {
		if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
			if (errno == EWOULDBLOCK) {
				return true;
			}
			// need to release lock and remove the file
		} else {
			flock(fd, LOCK_UN);
		}
	}
	return false;
}

RitexAdapter::~RitexAdapter() {
	if (m_pCmdLineParser != NULL) {
		delete m_pCmdLineParser;
		m_pCmdLineParser = NULL;
	}
}



int RitexAdapter::DaemonLoop() {
	/*
	 * -- Before enter to the main loop
	 * 1. Create communication channel to get commands from host process
	 * 2. Build offset table for parameters enabled in DB
	 * 3. Prepare SQL query for storing data to data DB
	 * 4. Open TTY to device as very last step
	 *
	 * -- In the loop
	 * 1. wait for ping-pong cycle. set window_opened = true
	 * 2. push data got in response to the DB-writing thread
	 * 3. check if there is pending command and send if any
	 */


//	int fd = OpenCommPort("/dev/ttyUSB0", 9600);
//
//	if (fd < 0) {
//		return -1;
//	}
//
//	// TODO: crate state machine
//	while(true) {
//		unsigned char address;
//		unsigned short length;
//		unsigned char command;
//		unsigned char* buffer;
//		bool isBadPacket = false;
//		int result;
//
//		isBadPacket = false;
//
//		//sync to beginig of the packet
//		do {
//			result = read(fd, &address, 1);
//		} while (result == 1 && address != 0x36);
//
//		result = read(fd,&length, 2); //packet length
//
//		result = read(fd, &command, 1); //command ID
//
//		// simple check
//		switch(command) {
//		case 0x84: //1
//		case 0x94: //1
//		case 0x88: //1
//		case 0x9C: //1
//		case 0xA4: //1
//			if (length != 1) {
//				isBadPacket = true;
//			}
//			break;
//		case 0x8C: //2
//		case 0x80: //2
//		case 0xA0: //2
//			if (length != 2) {
//				isBadPacket = true;
//			}
//			break;
//		case 0x90: //4
//			if (length != 4) {
//				isBadPacket = true;
//			}
//			break;
//		case 0x98: //7
//			if (length != 7) {
//				isBadPacket = true;
//			}
//			break;
//		//REPLYS
//		case 0x64: //0x28
//		case 0x68: //0x0213
//		case 0x6C: //0x2E
//		case 0x0B: // 2 // ACK
//		case 0x70: // 5
//			break;
//		default:
//			isBadPacket = true;
//			break;
//		}
//
//		if(isBadPacket) {
//			continue;
//		}
//
//		//we passed sanity check. probably good packet.
//		// read 'length' bytes +2 bytes CRC
//		buffer = new unsigned char[length + 2]; // + 2 for CRC
//		result = read(fd, buffer, length + 2);
//
//		//TODO: check CRC
//		//TODO: pass collected data to DB thread
//
//
//	}

//	int a = 1;
//	while (1) {
//		::sleep(3);
//		::syslog(LOG_ERR, "I'm daemon running for %d sec.", a * 3);
//		a++;
//	}
	return 0;
}


/*
 * -cmd
 * 24|Изменить уставку|Команды управления
~1|Изменить уставку|list|1~Число оборотов ВД|2~Ток ВД при перегрузе|3~Защита по перегрузу|4~Ток ВД при недогрузе|5~Защита по недогрузу
~2|Значение уставки|float
2|Команда управления|Команды управления ВД
~1|Команды управления ВД|list|7~Левое вращения ВД|8~Правое вращения ВД|9~Включить ВД|10~начать тарировку|11~Сброс ВД|12~Выключить ВД
 */
#include "EventLoggerThread.h"
int RitexAdapter::ParentLoop(bool isCommOk) {
	printf("We are in parent process PID: %d\n", ::getpid());
	printf("Exiting parent: PID: %d\n", ::getpid());
	return 0;
}

