#ifndef STRING_FILTER_H
#define STRING_FILTER_H

#include "filter.h"



class StringFilter : public Filter
{
	QString value;
	
public:
	StringFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(const QString& value);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const;
	static unique_ptr<StringFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // STRING_FILTER_H
