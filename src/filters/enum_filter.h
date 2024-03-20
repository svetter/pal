#ifndef ENUM_FILTER_H
#define ENUM_FILTER_H

#include "filter.h"



class EnumFilter : public Filter
{
	int value;
	
public:
	EnumFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(int value);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // ENUM_FILTER_H
