/*
 * IAdapter.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef IADAPTER_H_
#define IADAPTER_H_

#include "EventLoggerThread.h"
#include "DataLoggerThread.h"
#include <map>
#include "ParameterFilter.h"
#include <malloc.h>

enum eListValueType {
	LIST_VALUE_INT,
	LIST_VALUE_STRING
};

enum eParamType {
	PARAM_TYPE_INT,
	PARAM_TYPE_FLOAT,
	PARAM_TYPE_STRING,
	PARAM_TYPE_LIST
};

struct additional_parameter_t {
	eParamType type;
	union {
		struct _float {
			float min;
			float max;
		} f;
		struct _int {
			int min;
			int max;
		} i;
		struct _list {
			const char* const* data;
			int size;
			eListValueType dataType;
		} list;
		char* str;
	} value;

	virtual ~additional_parameter_t() {
		if(type == PARAM_TYPE_STRING) {
			free(value.str);
		}
	}
};

typedef std::map<std::string, struct additional_parameter_t*> additional_parameter_map_t;

class IAdapter {
public:
public:
	virtual const std::string& getName() = 0;
	virtual const std::string& getVersion() = 0;
	virtual const std::string& getDescription() = 0;
	virtual EventLoggerThread* getEventLogger() = 0;
	virtual DataLoggerThread* getDataLogger() = 0;
	virtual additional_parameter_map_t& GetAdditionalParameterMap() = 0;
	virtual const ParameterFilter& GetParameterFilter() const = 0;
//protected:
	virtual ~IAdapter() {};
};



#endif /* IADAPTER_H_ */
