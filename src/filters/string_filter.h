#ifndef STRING_FILTER_H
#define STRING_FILTER_H

#include "filter.h"



class StringFilter : public Filter
{
	QString value;
	
public:
	StringFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
	virtual ~StringFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static StringFilter* decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding);
	
	friend class Filter;
	friend class StringFilterBox;
};



#endif // STRING_FILTER_H