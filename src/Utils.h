/*
 * Utils.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <time.h>
#include <string>

class Utils {
private:
	Utils() {};
public:
	virtual ~Utils() {};
	static std::string TimeToString(time_t time);
};

#endif /* UTILS_H_ */
