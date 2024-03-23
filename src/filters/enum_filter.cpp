#include "enum_filter.h"

#include "src/filters/filter_widgets/enum_filter_box.h"



EnumFilter::EnumFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Enum, tableToFilter, columnToFilterBy, FilterFoldOp(-1), name),
	value(0)
{}



bool EnumFilter::evaluate(const QVariant& rawRowValue) const
{
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<int>());
		const int convertedValue = rawRowValue.toInt();
		const bool match = convertedValue == value;
		
		return match != isInverted();
	}
}



FilterBox* EnumFilter::createFilterBox(QWidget* parent)
{
	const QStringList& entries = *columnToFilterBy.enumNames;
	return new EnumFilterBox(parent, name, entries, *this);
}



QStringList EnumFilter::encodeTypeSpecific() const
{
	return {
		encodeInt("value", value)
	};
}

EnumFilter* EnumFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const int value = decodeInt(restOfEncoding, "value", ok, true);
	if (!ok) return nullptr;
	
	EnumFilter* const filter = new EnumFilter(tableToFilter, columnToFilterBy, name);
	filter->value = value;
	
	return filter;
}
