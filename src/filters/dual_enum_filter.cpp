#include "dual_enum_filter.h"

#include "src/filters/filter_widgets/dual_enum_filter_box.h"



DualEnumFilter::DualEnumFilter(const NormalTable& tableToFilter, const Column& discerningColumnToFilterBy, const QString& name) :
	Filter(DualEnum, tableToFilter, discerningColumnToFilterBy, name),
	dependentColumnToFilterBy(discerningColumnToFilterBy.table.getColumnByIndex(discerningColumnToFilterBy.getIndex() + 1)),
	discerningValue(-1),
	dependentValue(-1)
{
	assert(discerningColumnToFilterBy.enumNameLists == dependentColumnToFilterBy.enumNameLists);
}



void DualEnumFilter::setValues(int discerningValue, int dependentValue)
{
	this->discerningValue	= discerningValue;
	this->dependentValue	= dependentValue;
}



unique_ptr<FilterBox> DualEnumFilter::getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	DualEnumFilter* const castPointer = (DualEnumFilter*) thisFilter.release();
	unique_ptr<DualEnumFilter> castUnique = unique_ptr<DualEnumFilter>(castPointer);
	
	const QList<QPair<QString, QStringList>>& entries = *columnToFilterBy.enumNameLists;
	return make_unique<DualEnumFilterBox>(parent, name, entries, std::move(castUnique));
}



QStringList DualEnumFilter::encodeTypeSpecific() const
{
	return {
		encodeInt("discerningValue",	discerningValue),
		encodeInt("dependentValue",		dependentValue)
	};
}

unique_ptr<DualEnumFilter> DualEnumFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const int discerningValue = decodeInt(restOfEncoding, "discerningValue", ok);
	if (!ok) return nullptr;
	const int dependentValue = decodeInt(restOfEncoding, "dependentValue", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<DualEnumFilter> filter = make_unique<DualEnumFilter>(tableToFilter, columnToFilterBy, name);
	filter->discerningValue = discerningValue;
	filter->dependentValue = dependentValue;
	
	return filter;
}
