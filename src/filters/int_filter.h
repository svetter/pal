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
	IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, int classIncrement, int classesMinValue, int classesMaxValue);
	
	void setMinMax(int min, int max);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const;
	static unique_ptr<IntFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // INT_FILTER_H
