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
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual unique_ptr<FilterBox> createFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static unique_ptr<DualEnumFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // DUAL_ENUM_FILTER_H
