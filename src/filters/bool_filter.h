#ifndef BOOL_FILTER_H
#define BOOL_FILTER_H

#include "filter.h"



class BoolFilter : public Filter
{
	bool value;
	
public:
	BoolFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
	virtual ~BoolFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static BoolFilter* decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding);
	
	friend class Filter;
	friend class BoolFilterBox;
};



#endif // BOOL_FILTER_H
