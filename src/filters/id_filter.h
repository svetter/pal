#ifndef ID_FILTER_H
#define ID_FILTER_H

#include "filter.h"



class IDFilter : public Filter
{
	ItemID value;
	
public:
	IDFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(const ItemID& value);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // ID_FILTER_H
