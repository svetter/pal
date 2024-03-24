#ifndef ENUM_FILTER_H
#define ENUM_FILTER_H

#include "filter.h"



class EnumFilter : public Filter
{
	int value;
	
public:
	EnumFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	virtual ~EnumFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static EnumFilter* decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
	friend class EnumFilterBox;
};



#endif // ENUM_FILTER_H
