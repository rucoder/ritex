/*
 * AdapterParameter.cpp
 *
 *  Created on: Apr 21, 2013
 *      Author: ruinmmal
 */

#include "AdapterParameter.h"
#include <stdio.h>

AdapterParameter::AdapterParameter(int id, std::string name, bool isOverridable, std::string args, bool isEventDriven, void* controllerData)
	: m_Id(id), m_Name(name), m_bEventDriven(isEventDriven), m_bIsOverridable(isOverridable), m_paramArgs(args), m_ControllerData(controllerData)
{
}

AdapterParameter::~AdapterParameter() {
	m_ValueRangeMap.clear();
}

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


