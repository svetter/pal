#ifndef DUAL_ENUM_FILTER_H
#define DUAL_ENUM_FILTER_H

#include "filter.h"



class DualEnumFilter : public Filter
{
	const Column& dependentColumnToFilterBy;
	
	int discerningValue;
	int dependentValue;
	
public:
	DualEnumFilter(const NormalTable& tableToFilter, const Column& discerningColumnToFilterBy, const QString& name);
	
	void setValues(int discerningValue, int dependentValue);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const;
	static unique_ptr<DualEnumFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // DUAL_ENUM_FILTER_H
