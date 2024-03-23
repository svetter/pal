#include "string_filter.h"

#include "src/filters/filter_widgets/string_filter_box.h"



StringFilter::StringFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name) :
	Filter(String, tableToFilter, columnToFilterBy, foldOp, name),
	value("")
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
	return new StringFilterBox(parent, name, *this);
}



QStringList StringFilter::encodeTypeSpecific() const
{
	return {
		encodeString("value", value)
	};
}

StringFilter* StringFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const QString value = decodeString(restOfEncoding, "value", ok, true);
	if (!ok) return nullptr;
	
	StringFilter* const filter = new StringFilter(tableToFilter, columnToFilterBy, foldOp, name);
	filter->value = value;
	
	return filter;
}
