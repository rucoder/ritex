/*
 * Log.cpp
 *
 *  Created on: May 16, 2013
 *      Author: ruinmmal
 */
#include <string>

#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include "Utils.h"
#include "Log.h"
#include <fcntl.h>

static std::string gContext("C");
static int gLogfd = -1;

static char buffer[2048];

void Log(std::string format, ...)
{
	va_list vl, vl1;
	va_start(vl, format);
	va_start(vl1, format);
	std::string s = std::string("[") + gContext + ":" + itoa(getpid()) + "] " + format;
	vsyslog(LOG_ERR, s.c_str(), vl);
	if(gLogfd != -1) {
		if(s[s.length() - 1] != '\n') {
			s += std::string("\n");
		}
		int size = vsnprintf(buffer, 2048, s.c_str(), vl1);
		write(gLogfd, buffer, size);
	}
	va_end(vl);
	va_end(vl1);
}

void SetLogContext(Adapter::eExecutionContext context) {
	switch(context) {
	case Adapter::CONTEXT_DAEMON:
		gContext = std::string("D");
		break;
	case Adapter::CONTEXT_PARENT:
		gContext = std::string("C");
		break;
	case Adapter::CONTEXT_ERROR:
		gContext = std::string("E");
		break;
	}
}

void InitLog() {
	gLogfd = open("/tmp/Ritex.log", O_APPEND|O_WRONLY|O_CREAT, 0666);
}

int GetLogFd() {
	return gLogfd;
}




