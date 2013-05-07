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
#include <sstream>
#include <vector>

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

std::string itoa(int i);



class Utils {
private:
	Utils() {};
public:
	virtual ~Utils() {};
	static std::string TimeToString(time_t time);
    //CRC16
    static unsigned short Crc16(unsigned short crcInit, unsigned char byte);
    static unsigned short Crc16(unsigned short crcInit, unsigned char buffer[], int size);
    static void _do_backtrace();

    static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }


    static std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, elems);
        return elems;
    }

};

#endif /* UTILS_H_ */
