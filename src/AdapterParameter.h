/*
 * AdapterParameter.h
 *
 *  Created on: Apr 21, 2013
 *      Author: ruinmmal
 */

#ifndef ADAPTERPARAMETER_H_
#define ADAPTERPARAMETER_H_

#include <string>
#include <map>



class AdapterParameter {
private:
//	bool m_bCanModify;
	int m_Id;
	std::string m_Name;
	bool m_bEventDriven;
	bool m_bIsOverridable;
	std::string m_paramArgs;
	// only applicable if m_bEventDriven == true
	std::map<std::string, float> m_ValueRangeMap;
public:
	AdapterParameter();
	AdapterParameter(int id, const char* name, bool isOverridable, char* args, bool isEventDriven = false);
	virtual ~AdapterParameter();

	bool SetValueRange(std::string name, float value);

	bool GetValueRange(std::string name, float& value);

//	std::string FormatString();
	int GetId() { return m_Id; };
	std::string GetName() { return m_Name; };
	bool isEventDriven() { return m_bEventDriven; };
	bool isOverridable() { return m_bIsOverridable; };
	std::string GetArgs() { return m_paramArgs; };
};

#endif /* ADAPTERPARAMETER_H_ */
