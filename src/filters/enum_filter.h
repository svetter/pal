#ifndef ENUM_FILTER_H
#define ENUM_FILTER_H

#include "filter.h"



class EnumFilter : public Filter
{
	int value;
	
public:
	EnumFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(int value);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const;
	static unique_ptr<EnumFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // ENUM_FILTER_H
