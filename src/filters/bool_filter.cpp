#include "bool_filter.h"

#include "src/filters/filter_widgets/bool_filter_box.h"



BoolFilter::BoolFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Bit, tableToFilter, columnToFilterBy, FilterFoldOp(-1), name),
	value(true)
{}

BoolFilter::~BoolFilter()
{}



bool BoolFilter::evaluate(const QVariant& rawRowValue) const
{
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<bool>());
		const bool convertedValue = rawRowValue.toBool();
		const bool match = convertedValue == value;
		
		return match != isInverted();
	}
}



FilterBox* BoolFilter::createFilterBox(QWidget* parent)
{
	return new BoolFilterBox(parent, name, *this);
}



QStringList BoolFilter::encodeTypeSpecific() const
{
	return {
		encodeBool("value", value)
	};
}

BoolFilter* BoolFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const bool value = decodeBool(restOfEncoding, "value", ok, true);
	if (!ok) return nullptr;
	
	BoolFilter* const filter = new BoolFilter(tableToFilter, columnToFilterBy, name);
	filter->value = value;
	
	return filter;
}
