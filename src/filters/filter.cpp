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

#include "filter.h"

#include "src/filters/int_filter.h"
#include "src/filters/id_filter.h"
#include "src/filters/enum_filter.h"
#include "src/filters/dual_enum_filter.h"
#include "src/filters/bool_filter.h"
#include "src/filters/string_filter.h"
#include "src/filters/date_filter.h"
#include "src/filters/time_filter.h"

#include <QDateTime>



Filter::Filter(DataType type, const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	type(type),
	tableToFilter(tableToFilter),
	columnToFilterBy(columnToFilterBy),
	uiName(uiName),
	enabled(true),
	inverted(false)
{}

Filter::~Filter()
{}



bool Filter::isEnabled() const
{
	return enabled;
}

bool Filter::isInverted() const
{
	return inverted;
}

void Filter::setEnabled(bool enabled)
{
	this->enabled = enabled;
}

void Filter::setInverted(bool inverted)
{
	this->inverted = inverted;
}



void Filter::applyToOrderBuffer(ViewOrderBuffer& viewOrderBuffer) const
{
	for (ViewRowIndex viewRow = ViewRowIndex(viewOrderBuffer.numRows() - 1); viewRow.isValid(); viewRow--) {
		const BufferRowIndex bufferRow = viewOrderBuffer.getBufferRowIndexForViewRow(viewRow);
		
		// Get raw value
		QVariant rawRowValue;
		if (type == ID) {
			const QSet<ValidItemID> ids = columnToFilterBy.computeIDsAt(bufferRow);
			QVariantList idsAsVariants = QVariantList();
			for (const ValidItemID& id : ids) {
				idsAsVariants.append(id.asQVariant());
			}
			rawRowValue = QVariant(idsAsVariants);
		} else {
			rawRowValue = columnToFilterBy.getRawValueAt(bufferRow);
		}
		
		// Evaluate filter
		const bool pass = evaluate(rawRowValue);
		
		// Remove row if no match
		if (!pass) {
			viewOrderBuffer.removeViewRow(viewRow);
		}
	}
}



QString Filter::encodeToString(QList<const Filter*> filters)
{
	QStringList entries = QStringList();
	for (const Filter* const filter : filters) {
		entries.append(filter->encodeSingleFilterToString());
	}
	const QString encodedFilters = entries.join(",");
	return encodedFilters;
}

QString Filter::encodeSingleFilterToString() const
{
	const QString header = DataTypeNames::getName(type) + "Filter(";
	
	QStringList parameters = QStringList();
	parameters += encodeString	("tableToFilter_name",			tableToFilter.name);
	parameters += encodeString	("columnToFilterBy_table_name",	columnToFilterBy.table.name);
	parameters += encodeString	("columnToFilterBy_name",		columnToFilterBy.name);
	parameters += encodeString	("uiName",						uiName);
	parameters += encodeBool	("enabled",						enabled);
	parameters += encodeBool	("inverted",					inverted);
	parameters += encodeTypeSpecific();
	
	return header + parameters.join(",") + ",)";
}

QList<Filter*> Filter::decodeFromString(const QString& encoded, const ItemTypesHandler& typesHandler)
{
	QString restOfEncoding = encoded;
	QList<Filter*> filters = QList<Filter*>();
	
	while (!restOfEncoding.isEmpty()) {
		Filter* const parsedFilter = decodeSingleFilterFromString(restOfEncoding, typesHandler);
		if (!parsedFilter) {
			qDebug() << "WARNING: Aborted parsing filters from string:" << encoded;
			break;
		}
		
		filters.append(parsedFilter);
		
		if (restOfEncoding.isEmpty()) break;
		if (!restOfEncoding.startsWith(",")) {
			qDebug() << "WARNING: Aborted parsing filters from string:" << encoded;
			break;
		}
		restOfEncoding.remove(0, 1);
	}
	
	return filters;
}

Filter* Filter::decodeSingleFilterFromString(QString& restOfEncoding, const ItemTypesHandler& typesHandler)
{
	bool ok = false;
	
	const DataType type = decodeHeader<DataType>(restOfEncoding, "Filter", DataTypeNames::getType, ok);
	if (!ok) return nullptr;
	
	const CompositeTable* tableToFilter = decodeCompTableIdentity(restOfEncoding, "tableToFilter_name", typesHandler, ok);
	if (!ok) return nullptr;
	
	const CompositeColumn* columnToFilterBy = decodeCompColumnIdentity(restOfEncoding, "columnToFilterBy_table_name", "columnToFilterBy_name", typesHandler, ok);
	if (!ok) return nullptr;
	
	const QString uiName = decodeString(restOfEncoding, "uiName", ok);
	if (!ok) return nullptr;
	
	const bool enabled = decodeBool(restOfEncoding, "enabled", ok);
	if (!ok) return nullptr;
	
	const bool inverted = decodeBool(restOfEncoding, "inverted", ok);
	if (!ok) return nullptr;
	
	Filter* decodedFilter = nullptr;
	
	switch (type) {
	case Integer:	decodedFilter =      IntFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	case ID:		decodedFilter =       IDFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	case Enum:		decodedFilter =     EnumFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	case DualEnum:	decodedFilter = DualEnumFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	case Bit:		decodedFilter =     BoolFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	case String:	decodedFilter =   StringFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	case Date:		decodedFilter =     DateFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	case Time:		decodedFilter =     TimeFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, uiName, restOfEncoding);	break;
	default: assert(false);
	}
	if (!decodedFilter) return nullptr;
	
	const QString endDelimiter = ")";
	if (!restOfEncoding.startsWith(endDelimiter)) {
		delete decodedFilter;
		return nullptr;
	}
	restOfEncoding.remove(0, endDelimiter.size());
	
	decodedFilter->enabled = enabled;
	decodedFilter->inverted = inverted;
	
	return decodedFilter;
}
