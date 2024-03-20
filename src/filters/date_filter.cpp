#include "date_filter.h"

#include "src/filters/filter_widgets/date_filter_box.h"



DateFilter::DateFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Date, tableToFilter, columnToFilterBy, name),
	min(QDate()),
	max(QDate())
{}



void DateFilter::setValues(const QDate& min, const QDate& max)
{
	this->min = min;
	this->max = max;
}



FilterBox* DateFilter::getFilterBox(QWidget* parent) const
{
	return new DateFilterBox(parent, name);
}
