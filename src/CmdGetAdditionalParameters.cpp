/*
 * CmdGetSupportedParameters.cpp
 *
 *  Created on: Apr 27, 2013
 *      Author: ruinmmal
 */

#include "CmdGetAdditionalParameters.h"
#include <stdio.h>
#include "IAdapter.h"

CmdGetAdditionalParameters::CmdGetAdditionalParameters(IAdapter * pAdapter)
	: DeviceCommand(false), m_pAdapter(pAdapter)
{
	// TODO Auto-generated constructor stub

}

CmdGetAdditionalParameters::~CmdGetAdditionalParameters() {
	// TODO Auto-generated destructor stub
}

bool CmdGetAdditionalParameters::Execute()
{
	std::map<std::string, struct additional_parameter_t*> params = m_pAdapter->GetAdditionalParameterMap();

	for(std::map<std::string, struct additional_parameter_t*>::iterator itr = params.begin(); itr != params.end(); itr++) {
		//name
		printf("%s", itr->first.c_str());
		//type
		switch(itr->second->type) {
			case PARAM_TYPE_INT:
				printf("|int|%d|%d\n", itr->second->value.i.min, itr->second->value.i.max);
				break;
			case PARAM_TYPE_FLOAT:
				printf("|float|%g|%g\n", itr->second->value.f.min, itr->second->value.f.max);
				break;
			case PARAM_TYPE_STRING:
				printf("|string|%s\n", itr->second->value.str);
				break;
			case PARAM_TYPE_LIST:
				printf("|list");
				for(int i = 0; i < itr->second->value.list.size; i++) {
					printf("|%s", itr->second->value.list.data[i]);
				}
				printf("\n");
				break;
		}
	}
	return true;
}


