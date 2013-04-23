/*
 * AdapterParameter.cpp
 *
 *  Created on: Apr 21, 2013
 *      Author: ruinmmal
 */

#include "AdapterParameter.h"
#include <stdio.h>

AdapterParameter::AdapterParameter(int id, const char* name, bool isOverridable, char* args, bool isEventDriven)
	: m_Id(id), m_Name(name), m_bEventDriven(isEventDriven), m_paramArgs(args), m_bIsOverridable(isOverridable)
{
}

AdapterParameter::~AdapterParameter() {
	m_ValueRangeMap.clear();
}

//std::string AdapterParameter::FormatString()
//{
//	char* buffer = new char[256];
//	std::string str = "";
//	snprintf(buffer,256,"%d|%s|%d|%d|%s|%d", m_Id, m_Name.c_str(), m_bCanModify ? 1 : 0,
//			m_bIsOverridable ? 1 : 0, m_paramArgs.c_str(), m_bEventDriven ? 1:0);
//	str+=buffer;
//	if(m_bEventDriven) {
////	    for(std::list<struct _tag_value_range*>::iterator itr = m_valueRangeList.begin(); itr != m_valueRangeList.end(); itr++)
////	    {
////	    	//TODO:
////	    }
//	}
//	return str;
//}


bool AdapterParameter::SetValueRange(std::string name, float value)
{
	m_bEventDriven = true;
	m_ValueRangeMap[name] = value;
	return true;
}

bool AdapterParameter::GetValueRange(std::string name, float& value)
{
	std::map<std::string, float>::iterator it = m_ValueRangeMap.find(name);

	if (it == m_ValueRangeMap.end())
		return false;

	value = it->second;

	return true;
}


