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
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // INT_FILTER_H
