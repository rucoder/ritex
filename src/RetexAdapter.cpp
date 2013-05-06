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

const char* RitexAdapter::m_commDevices[] = {
	"/dev/ttySC0",
	"/dev/ttySC1",
	"/dev/ttySC2",
	"/dev/ttySC3",
	"/dev/ttySC4",
	"/dev/tnt0",
	"/dev/tnt1",
	"/dev/ttyUSB0",
	"/dev/ttyUSB1"
};


const char* RitexAdapter::m_commSpeed[] = {
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
	return 0;
}



#include "EventLoggerThread.h"
int RitexAdapter::ParentLoop(bool isCommOk) {
	printf("We are in parent process PID: %d\n", ::getpid());
	printf("Exiting parent: PID: %d\n", ::getpid());
	return 0;
}

