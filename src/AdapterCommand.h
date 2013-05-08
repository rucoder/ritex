/*
 * AdapterCommand.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef ADAPTERCOMMAND_H_
#define ADAPTERCOMMAND_H_

#include <string>
#include <vector>
typedef enum
{
	ARG_TYPE_INT,
	ARG_TYPE_FLOAT,
	ARG_TYPE_STRING,
	ARG_TYPE_LIST,
	ARG_TYPE_DATETIME
} arg_type_t;

class AdapterCommandArgParam
{
public:
	std::string m_name;
	void* m_deviceData;
	AdapterCommandArgParam(std::string name, void* data = NULL)
		: m_name(name), m_deviceData(data)
	{

	}

};

class AdapterCommandArg
{
public:
	std::string m_name;
	arg_type_t m_type;
	int m_int_min_val, m_int_max_val;
	float m_float_min_val, m_float_max_val;
	std::string m_string_val;
	std::vector<AdapterCommandArgParam*> m_list_val;
	AdapterCommandArg(std::string name, int minval, int maxval) {
		m_name = name;
		m_int_min_val = minval;
		m_int_max_val = maxval;
		m_type = ARG_TYPE_INT;
	}
	AdapterCommandArg(std::string name, float minval, float maxval) {
		m_name = name;
		m_float_min_val = minval;
		m_float_max_val = maxval;
		m_type = ARG_TYPE_FLOAT;
	}
	AdapterCommandArg(std::string name, std::vector<AdapterCommandArgParam*> val) {
		m_name = name;
		m_list_val = val;
		m_type = ARG_TYPE_LIST;
	}
	virtual ~AdapterCommandArg() {
		for(unsigned int i = 0; i < m_list_val.size(); i++) {
			delete m_list_val[i];
		}
	}
};

class AdapterCommand {
public:
	int m_cmdId;
	std::string m_name;
	std::string m_description;
	std::vector<AdapterCommandArg*> m_args;
public:
	AdapterCommand(int id, std::string name, std::string description);
	void AddArgument(std::string name, std::vector<AdapterCommandArgParam*> list) {
		m_args.push_back(new AdapterCommandArg(name, list));
	}
	void AddArgument(std::string name, float min, float max) {
		m_args.push_back(new AdapterCommandArg(name, min, max));
	}
	virtual ~AdapterCommand();
	std::vector<AdapterCommandArg*>& GetAgrList() { return m_args; }
};

#endif /* ADAPTERCOMMAND_H_ */
