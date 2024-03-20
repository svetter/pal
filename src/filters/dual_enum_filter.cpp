#include "dual_enum_filter.h"

#include "src/filters/filter_widgets/dual_enum_filter_box.h"



DualEnumFilter::DualEnumFilter(const NormalTable& tableToFilter, const Column& discerningColumnToFilterBy, const Column& dependentColumnToFilterBy, const QString& name) :
	Filter(DualEnum, tableToFilter, discerningColumnToFilterBy, name),
	dependentColumnToFilterBy(dependentColumnToFilterBy),
	discerningValue(-1),
	dependentValue(-1)
{}



void DualEnumFilter::setValues(int discerningValue, int dependentValue)
{
	this->discerningValue	= discerningValue;
	this->dependentValue	= dependentValue;
}



FilterBox* DualEnumFilter::getFilterBox(QWidget* parent) const
{
	const QList<QPair<QString, QStringList>>& entries = *columnToFilterBy.enumNameLists;
	return new DualEnumFilterBox(parent, name, entries);
}
