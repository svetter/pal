#ifndef BOOL_FILTER_H
#define BOOL_FILTER_H

#include "filter.h"



class BoolFilter : public Filter
{
	bool value;
	
public:
	BoolFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValue(bool value);
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual unique_ptr<FilterBox> createFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static unique_ptr<BoolFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // BOOL_FILTER_H
