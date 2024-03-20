#include "string_filter.h"

#include "src/filters/filter_widgets/string_filter_box.h"



StringFilter::StringFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(String, tableToFilter, columnToFilterBy, name),
	value(QString())
{}



void StringFilter::setValue(const QString& value)
{
	this->value = value;
}



FilterBox* StringFilter::getFilterBox(QWidget* parent) const
{
	return new StringFilterBox(parent, name);
}
