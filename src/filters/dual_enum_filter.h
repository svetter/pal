#ifndef DUAL_ENUM_FILTER_H
#define DUAL_ENUM_FILTER_H

#include "filter.h"



class DualEnumFilter : public Filter
{
	int discerningValue;
	int dependentValue;
	
public:
	DualEnumFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
	virtual ~DualEnumFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static DualEnumFilter* decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding);
	
	friend class Filter;
	friend class DualEnumFilterBox;
};



#endif // DUAL_ENUM_FILTER_H
