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
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static BoolFilter* decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // BOOL_FILTER_H
