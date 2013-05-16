/*
 * Log.h
 *
 *  Created on: May 16, 2013
 *      Author: ruinmmal
 */

#ifndef LOG_H_
#define LOG_H_
#include "Adapter.h"

void InitLog();
int GetLogFd();
void Log(std::string format, ...);
void SetLogContext(Adapter::eExecutionContext context);

#endif /* LOG_H_ */
