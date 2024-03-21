#ifndef ID_FILTER_H
#define ID_FILTER_H

#include "filter.h"



class IDFilter : public Filter
{
	ItemID value;
	
public:
	IDFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(const ItemID& value);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const;
	static unique_ptr<IDFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // ID_FILTER_H
