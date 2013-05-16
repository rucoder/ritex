/*
 * ParameterFilter.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef PARAMETERFILTER_H_
#define PARAMETERFILTER_H_

#include <vector>

typedef struct parameter_filter_entry_t {
	int m_channelId;
	int m_paramId;
	parameter_filter_entry_t(int channel, int param)
		: m_channelId(channel), m_paramId(param)
	{

	}
} ParaneterFilterItem;


class ParameterFilter {
private:
	std::vector<struct parameter_filter_entry_t> m_items;
public:
	ParameterFilter() {};
	virtual ~ParameterFilter() {
		m_items.clear();
	}
	void AddItem(int channel, int param) {
		m_items.push_back(ParaneterFilterItem(channel, param));
	}
	int GetSize() { return m_items.size(); }
	const ParaneterFilterItem&  operator[](int index) const {
		return m_items[index];
	}

	int FindChannel(int param) {
		for(unsigned int i = 0; i < m_items.size(); i++) {
			if(m_items[i].m_paramId == param) {
				return m_items[i].m_channelId;
			}
		}
		return -1;
	}
};

#endif /* PARAMETERFILTER_H_ */
