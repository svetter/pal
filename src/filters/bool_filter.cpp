/*
 * Copyright 2023-2024 Simon Vetter
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
