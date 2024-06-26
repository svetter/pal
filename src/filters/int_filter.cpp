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

#include "int_filter.h"

#include "src/filters/filter_widgets/int_filter_box.h"
#include "src/filters/filter_widgets/int_class_filter_box.h"



IntFilter::IntFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(Integer, tableToFilter, columnToFilterBy, uiName),
	useClasses(false),
	classIncrement(-1),
	classesMinValue(-1),
	classesMaxValue(-1),
	min(0),
	max(1)
{}

IntFilter::IntFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, int classIncrement, int classesMinValue, int classesMaxValue) :
	Filter(Integer, tableToFilter, columnToFilterBy, uiName),
	useClasses(true),
	classIncrement(classIncrement),
	classesMinValue(classesMinValue),
	classesMaxValue(classesMaxValue),
	min(0),
	max(0)
{}

IntFilter::~IntFilter()
{}



bool IntFilter::evaluate(const QVariant& rawRowValue) const
{
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<int>());
		const int convertedValue = rawRowValue.toInt();
		const bool match = convertedValue >= min && convertedValue <= max;
		
		return match != isInverted();
	}
}



FilterBox* IntFilter::createFilterBox(QWidget* parent)
{
	if (useClasses) {
		return new IntClassFilterBox(parent, uiName, classIncrement, classesMinValue, classesMaxValue, *this);
	} else {
		return new IntFilterBox(parent, uiName, *this);
	}
}



QStringList IntFilter::encodeTypeSpecific() const
{
	return {
		encodeBool("useClasses",		useClasses),
		encodeInt("classIncrement",		classIncrement),
		encodeInt("classesMinValue",	classesMinValue),
		encodeInt("classesMaxValue",	classesMaxValue),
		encodeInt("min",				min),
		encodeInt("max",				max)
	};
}

IntFilter* IntFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const bool useClasses = decodeBool(restOfEncoding, "useClasses", ok);
	if (!ok)
		return nullptr;
	const int classIncrement = decodeInt(restOfEncoding, "classIncrement", ok);
	if (!ok && useClasses)
		return nullptr;
	const int classesMinValue = decodeInt(restOfEncoding, "classesMinValue", ok);
	if (!ok && useClasses)
		return nullptr;
	const int classesMaxValue = decodeInt(restOfEncoding, "classesMaxValue", ok);
	if (!ok && useClasses)
		return nullptr;
	const int min = decodeInt(restOfEncoding, "min", ok);
	if (!ok)
		return nullptr;
	const int max = decodeInt(restOfEncoding, "max", ok);
	if (!ok)
		return nullptr;
	
	IntFilter* filter = nullptr;
	if (useClasses) {
		filter = new IntFilter(tableToFilter, columnToFilterBy, uiName, classIncrement, classesMinValue, classesMaxValue);
	} else {
		filter = new IntFilter(tableToFilter, columnToFilterBy, uiName);
	}
	filter->min = min;
	filter->max = max;
	
	return filter;
}
