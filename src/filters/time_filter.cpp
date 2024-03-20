#include "time_filter.h"

#include "src/filters/filter_widgets/time_filter_box.h"



TimeFilter::TimeFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Time, tableToFilter, columnToFilterBy, name),
	min(QTime()),
	max(QTime())
{}



void TimeFilter::setValues(const QTime& min, const QTime& max)
{
	this->min = min;
	this->max = max;
}



FilterBox* TimeFilter::getFilterBox(QWidget* parent) const
{
	return new TimeFilterBox(parent, name);
}
