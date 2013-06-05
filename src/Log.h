/*
 * Log.h
 *
 *  Created on: May 16, 2013
 *      Author: ruinmmal
 */

#ifndef LOG_H_
#define LOG_H_

#include "Adapter.h"
#include <string>
class Adapter;
extern void InitLog();
extern int GetLogFd();
extern void Log(std::string format, ...);
extern void SetLogContext(Adapter::eExecutionContext context);

#endif /* LOG_H_ */
