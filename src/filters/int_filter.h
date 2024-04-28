#ifndef INT_FILTER_H
#define INT_FILTER_H

#include "filter.h"



class IntFilter : public Filter
{
public:
	const bool useClasses;
	const int classIncrement;
	const int classesMinValue;
	const int classesMaxValue;
	
private:
	int min;
	int max;
	
public:
	IntFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
	IntFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, int classIncrement, int classesMinValue, int classesMaxValue);
	virtual ~IntFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static IntFilter* decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding);
	
	friend class Filter;
	friend class IntFilterBox;
	friend class IntClassFilterBox;
};



#endif // INT_FILTER_H
