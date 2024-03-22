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
	IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name);
	IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name, int classIncrement, int classesMinValue, int classesMaxValue);
	
	void setMinMax(int min, int max);
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static IntFilter* decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // INT_FILTER_H
