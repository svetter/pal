#include "bool_filter.h"

#include "src/filters/filter_widgets/bool_filter_box.h"



BoolFilter::BoolFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(Bit, tableToFilter, columnToFilterBy, uiName),
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
	return new BoolFilterBox(parent, uiName, *this);
}



QStringList BoolFilter::encodeTypeSpecific() const
{
	return {
		encodeBool("value", value)
	};
}

BoolFilter* BoolFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const bool value = decodeBool(restOfEncoding, "value", ok);
	if (!ok) return nullptr;
	
	BoolFilter* const filter = new BoolFilter(tableToFilter, columnToFilterBy, uiName);
	filter->value = value;
	
	return filter;
}
