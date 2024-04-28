#include "dual_enum_filter.h"

#include "src/filters/filter_widgets/dual_enum_filter_box.h"



DualEnumFilter::DualEnumFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(DualEnum, tableToFilter, columnToFilterBy, uiName),
	discerningValue(0),
	dependentValue(0)
{}

DualEnumFilter::~DualEnumFilter()
{}



bool DualEnumFilter::evaluate(const QVariant& rawRowValue) const
{
	assert(discerningValue >= 0);
	assert(dependentValue  >= 0);
	assert(!(discerningValue == 0 && dependentValue > 0));
	
	/*                        ╔════════════════════════════════════════════════════════╗
	 *                        ║                 Values from filtered table             ║
	 * ╔══════════════════════╬───────────────┬────────┬─────────┬─────────┬───────────╢
	 * ║         Filter       ║ QVariant null │ {0, 0} │ {0, >0} │ {>0, 0} │ {>0, >0}  ║
	 * ╟───────────┬──────────╬═══════════════╪════════╪═════════╧═════════╪═══════════╣
	 * ║           │ { 0,  0} ║ true          │ true   │                   │ false     ║
	 * ║           ├──────────╫───────────────┴────────┘                   └───────────╢
	 * ║  Include  │ { 0, >0} ║                                                        ║
	 * ║    i.e.   ├──────────╫───────────────┬────────┐                   ┌───────────╢
	 * ║ !inverted │ {>0,  0} ║ false         │ false  │                   │ match1    ║
	 * ║           ├──────────╫───────────────┼────────┤                   ├───────────╢
	 * ║           │ {>0, >0} ║ false         │ false  │                   │ match1&2  ║
	 * ╟───────────┼──────────╫───────────────┼────────┤      invalid      ├───────────╢
	 * ║           │ { 0,  0} ║ false         │ false  │                   │ true      ║
	 * ║           ├──────────╫───────────────┴────────┘                   └───────────╢
	 * ║  Exclude  │ { 0, >0} ║                                                        ║
	 * ║    i.e.   ├──────────╫───────────────┬────────┐                   ┌───────────╢
	 * ║  inverted │ {>0,  0} ║ true          │ true   │                   │ !match1   ║
	 * ║           ├──────────╫───────────────┼────────┤                   ├───────────╢
	 * ║           │ {>0, >0} ║ true          │ true   │                   │ !match1&2 ║
	 * ╚═══════════╧══════════╩═══════════════╧════════╧═══════════════════╧═══════════╝
	 */
	
	if (rawRowValue.isNull()) {
		return (discerningValue == 0) != isInverted();
	}
	else {
		assert(rawRowValue.canConvert<QVariantList>());
		const QVariantList convertedList = rawRowValue.value<QVariantList>();
		
		if (convertedList.isEmpty()) {
			return (discerningValue == 0) != isInverted();
		}
		
		assert(convertedList.size() == 2);
		assert(convertedList.at(0).canConvert<int>());
		assert(convertedList.at(1).canConvert<int>());
		const int convertedDiscerning	= convertedList.at(0).toInt();
		const int convertedDependent	= convertedList.at(1).toInt();
		assert((convertedDiscerning > 0) == (convertedDependent > 0));
		
		if (convertedDiscerning == 0) {
			return (discerningValue == 0) != isInverted();
		}
		else {
			const bool discerningMatch = convertedDiscerning == discerningValue;
			const bool  dependentMatch = convertedDependent  == dependentValue;
			const bool bothMatch = discerningMatch && dependentMatch;
			
			if (dependentValue == 0) {
				return discerningMatch != isInverted();
			}
			else {
				return bothMatch != isInverted();
			}
		}
	}
}



FilterBox* DualEnumFilter::createFilterBox(QWidget* parent)
{
	const QList<QPair<QString, QStringList>>& entries = *columnToFilterBy.enumNameLists;
	return new DualEnumFilterBox(parent, uiName, entries, *this);
}



QStringList DualEnumFilter::encodeTypeSpecific() const
{
	return {
		encodeInt("discerningValue",	discerningValue),
		encodeInt("dependentValue",		dependentValue)
	};
}

DualEnumFilter* DualEnumFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const int discerningValue = decodeInt(restOfEncoding, "discerningValue", ok);
	if (!ok) return nullptr;
	const int dependentValue = decodeInt(restOfEncoding, "dependentValue", ok);
	if (!ok) return nullptr;
	
	DualEnumFilter* const filter = new DualEnumFilter(tableToFilter, columnToFilterBy, uiName);
	filter->discerningValue = discerningValue;
	filter->dependentValue = dependentValue;
	
	return filter;
}
