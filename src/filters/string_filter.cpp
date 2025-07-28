/*
 * Copyright 2023-2025 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

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
	if (value.isEmpty()) return true;
	
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
		
		if (convertedValue.isEmpty()) {
			return value.isEmpty() != isInverted();
		}
		else if (value.isEmpty()) {
			return isInverted();
		}
		else {
			const QStringList split = value.split(" ");
			bool containsAny = false;
			for (const QString& string : split) {
				if (convertedValue.contains(string, Qt::CaseInsensitive)) {
					containsAny = true;
					break;
				}
			}
			return containsAny != isInverted();
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
