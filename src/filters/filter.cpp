#include "filter.h"



Filter::Filter(DataType type, const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	type(type),
	tableToFilter(tableToFilter),
	columnToFilterBy(columnToFilterBy),
	name(name),
	inverted(false)
{}



bool Filter::isInverted() const
{
	return inverted;
}

void Filter::setInverted(bool inverted)
{
	this->inverted = inverted;
}
