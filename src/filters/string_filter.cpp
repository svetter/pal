#include "string_filter.h"

#include "src/filters/filter_widgets/string_filter_box.h"



StringFilter::StringFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(String, tableToFilter, columnToFilterBy, uiName),
	value("")
{}

StringFilter::~StringFilter()
{}



bool StringFilter::evaluate(const QVariant& rawRowValue) const
{
	assert(!value.isNull());
	
	/*                                ╔═════════════════════════════════════════════════╗
	 *                                ║           Value from filtered table             ║
	 * ╔══════════════════════════════╬───────────────┬──────────────┬──────────────────╢
	 * ║        Filter                ║ QVariant null │ Empty string │ Non-empty string ║
	 * ╟───────────┬──────────────────╬═══════════════╪══════════════╪══════════════════╣
	 * ║  Include  │     Empty string ║ true          │ true         │ false            ║
	 * ║    i.e.   ├──────────────────╫───────────────┼──────────────┼──────────────────╢
	 * ║ !inverted │ Non-empty string ║ false         │ false        │ contains         ║
	 * ╟───────────┼──────────────────╫───────────────┼──────────────┼──────────────────╢
	 * ║  Exclude  │     Empty string ║ false         │ false        │ true             ║
	 * ║    i.e.   ├──────────────────╫───────────────┼──────────────┼──────────────────╢
	 * ║  inverted │ Non-empty string ║ true          │ true         │ !contains        ║
	 * ╚═══════════╧══════════════════╩═══════════════╧══════════════╧══════════════════╝
	 */
	
	if (rawRowValue.isNull()) {
		return value.isEmpty() != isInverted();
	}
	else {
		assert(rawRowValue.canConvert<QString>());
		const QString convertedValue = rawRowValue.toString();
		assert(!convertedValue.isNull());
		
		if (convertedValue.isEmpty()) {
			return value.isEmpty() != isInverted();
		}
		else if (value.isEmpty()) {
			return isInverted();
		}
		else {
			const bool contains = convertedValue.contains(value, Qt::CaseInsensitive);
			return contains != isInverted();
		}
	}
}



FilterBox* StringFilter::createFilterBox(QWidget* parent)
{
	return new StringFilterBox(parent, uiName, *this);
}



QStringList StringFilter::encodeTypeSpecific() const
{
	return {
		encodeString("value", value)
	};
}

StringFilter* StringFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const QString value = decodeString(restOfEncoding, "value", ok);
	if (!ok) return nullptr;
	
	StringFilter* const filter = new StringFilter(tableToFilter, columnToFilterBy, uiName);
	filter->value = value;
	
	return filter;
}
