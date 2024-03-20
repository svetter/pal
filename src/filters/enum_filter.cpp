#include "enum_filter.h"

#include "src/filters/filter_widgets/enum_filter_box.h"



EnumFilter::EnumFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Enum, tableToFilter, columnToFilterBy, name),
	value(-1)
{}



void EnumFilter::setValue(int value)
{
	this->value = value;
}



FilterBox* EnumFilter::getFilterBox(QWidget* parent) const
{
	const QStringList& entries = *columnToFilterBy.enumNames;
	return new EnumFilterBox(parent, name, entries);
}
