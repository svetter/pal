#ifndef STRING_FILTER_H
#define STRING_FILTER_H

#include "filter.h"



class StringFilter : public Filter
{
	QString value;
	
public:
	StringFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name);
	
	void setValue(const QString& value);
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual unique_ptr<FilterBox> createFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static unique_ptr<StringFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // STRING_FILTER_H
