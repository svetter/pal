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



unique_ptr<FilterBox> EnumFilter::getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	EnumFilter* const castPointer = (EnumFilter*) thisFilter.release();
	unique_ptr<EnumFilter> castUnique = unique_ptr<EnumFilter>(castPointer);
	
	const QStringList& entries = *columnToFilterBy.enumNames;
	return make_unique<EnumFilterBox>(parent, name, entries, std::move(castUnique));
}



QStringList EnumFilter::encodeTypeSpecific() const
{
	return {
		encodeInt("value", value)
	};
}

unique_ptr<EnumFilter> EnumFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const int value = decodeInt(restOfEncoding, "value", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<EnumFilter> filter = make_unique<EnumFilter>(tableToFilter, columnToFilterBy, name);
	filter->value = value;
	
	return filter;
}
