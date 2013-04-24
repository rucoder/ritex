/*
 * Utils.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "Utils.h"

#include <stdio.h>

/*
 * Returns time in format YYYY-MM-DD HH:MM:SS
 */
std::string Utils::TimeToString(time_t time)
{
	char buffer[32];
	struct tm* tm = localtime(&time);
	snprintf(buffer,32,"%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return std::string(buffer);
}
