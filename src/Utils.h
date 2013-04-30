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

//need to swap byte if running on LE target (e.g. x86)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline unsigned short swap16(unsigned short x)
{
    return ( (x << 8) | (x >> 8) );
}
#define MSB(x) (((x) >> 8) & 0xFF)
#define LSB(x) ((x) & 0xFF)
#else
#define swap16(x) (x)
#define LSB(x) (((x) >> 8) & 0xFF)
#define MSB(x) ((x) & 0xFF)
#endif


class Utils {
private:
	Utils() {};
public:
	virtual ~Utils() {};
	static std::string TimeToString(time_t time);
    //CRC16
    static unsigned short Crc16(unsigned short crcInit, unsigned char byte);
    static unsigned short Crc16(unsigned short crcInit, unsigned char buffer[], int size);

};

#endif /* UTILS_H_ */
