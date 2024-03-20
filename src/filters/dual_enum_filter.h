#ifndef DUAL_ENUM_FILTER_H
#define DUAL_ENUM_FILTER_H

#include "filter.h"



class DualEnumFilter : public Filter
{
	const Column& dependentColumnToFilterBy;
	
	int discerningValue;
	int dependentValue;
	
public:
	DualEnumFilter(const NormalTable& tableToFilter, const Column& discerningColumnToFilterBy, const Column& dependentColumnToFilterBy, const QString& name);
	
	void setValues(int discerningValue, int dependentValue);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // DUAL_ENUM_FILTER_H
