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

#include "date_filter.h"

#include "src/filters/filter_widgets/date_filter_box.h"



DateFilter::DateFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(Date, tableToFilter, columnToFilterBy, uiName),
	min(QDate::currentDate()),
	max(QDate::currentDate())
{}

DateFilter::~DateFilter()
{}



bool DateFilter::evaluate(const QVariant& rawRowValue) const
{
	assert(!min.isNull());
	assert(!max.isNull());
	
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<QDate>() && rawRowValue.toDate().isValid());
		const QDate convertedValue = rawRowValue.toDate();
		const bool match = convertedValue >= min && convertedValue <= max;
		
		return match != isInverted();
	}
}



FilterBox* DateFilter::createFilterBox(QWidget* parent)
{
	return new DateFilterBox(parent, uiName, *this);
}



QStringList DateFilter::encodeTypeSpecific() const
{
	return {
		encodeDate("min", min),
		encodeDate("max", max)
	};
}

DateFilter* DateFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const QDate min = decodeDate(restOfEncoding, "min", ok);
	if (!ok) return nullptr;
	const QDate max = decodeDate(restOfEncoding, "max", ok);
	if (!ok) return nullptr;
	
	DateFilter* const filter = new DateFilter(tableToFilter, columnToFilterBy, uiName);
	filter->min = min;
	filter->max = max;
	
	return filter;
}
