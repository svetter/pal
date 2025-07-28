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

#include "enum_filter.h"

#include "src/filters/filter_widgets/enum_filter_box.h"



EnumFilter::EnumFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(Enum, tableToFilter, columnToFilterBy, uiName),
	value(0)
{}

EnumFilter::~EnumFilter()
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
	return new EnumFilterBox(parent, uiName, entries, *this);
}



QStringList EnumFilter::encodeTypeSpecific() const
{
	return {
		encodeInt("value", value)
	};
}

EnumFilter* EnumFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const int value = decodeInt(restOfEncoding, "value", ok);
	if (!ok) return nullptr;
	
	EnumFilter* const filter = new EnumFilter(tableToFilter, columnToFilterBy, uiName);
	filter->value = value;
	
	return filter;
}
