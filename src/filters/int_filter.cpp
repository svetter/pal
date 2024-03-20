#include "int_filter.h"

#include "src/filters/filter_widgets/int_filter_box.h"
#include "src/filters/filter_widgets/int_class_filter_box.h"



IntFilter::IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Integer, tableToFilter, columnToFilterBy, name),
	useClasses(false),
	classIncrement(-1),
	classesMinValue(-1),
	classesMaxValue(-1),
	min(0),
	max(0)
{}

IntFilter::IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, int classIncrement, int classesMinValue, int classesMaxValue) :
	Filter(Integer, tableToFilter, columnToFilterBy, name),
	useClasses(true),
	classIncrement(classIncrement),
	classesMinValue(classesMinValue),
	classesMaxValue(classesMaxValue),
	min(0),
	max(0)
{}



void IntFilter::setMinMax(int min, int max)
{
	this->min	= min;
	this->max	= max;
}



FilterBox* IntFilter::getFilterBox(QWidget* parent) const
{
	if (useClasses) {
		return new IntClassFilterBox(parent, name, classIncrement, classesMinValue, classesMaxValue);
	} else {
		return new IntFilterBox(parent, name);
	}
}
