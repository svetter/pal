#ifndef BOOL_FILTER_H
#define BOOL_FILTER_H

#include "filter.h"



class BoolFilter : public Filter
{
	bool value;
	
public:
	BoolFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(bool value);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // BOOL_FILTER_H
