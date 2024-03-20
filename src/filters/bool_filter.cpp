#include "bool_filter.h"

#include "src/filters/filter_widgets/bool_filter_box.h"



BoolFilter::BoolFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Bit, tableToFilter, columnToFilterBy, name),
	value(true)
{}



void BoolFilter::setValue(bool value)
{
	this->value = value;
}



FilterBox* BoolFilter::getFilterBox(QWidget* parent) const
{
	return new BoolFilterBox(parent, name);
}
