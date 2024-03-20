#ifndef STRING_FILTER_H
#define STRING_FILTER_H

#include "filter.h"



class StringFilter : public Filter
{
	QString value;
	
public:
	StringFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(const QString& value);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // STRING_FILTER_H
