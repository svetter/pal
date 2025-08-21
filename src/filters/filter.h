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

#ifndef FILTER_H
#define FILTER_H

#include "src/comp_tables/composite_table.h"
#include "src/settings/string_encoder.h"

class FilterBox;



class Filter : public StringEncoder
{
public:
	const DataType type;
	const CompositeTable& tableToFilter;
	const CompositeColumn& columnToFilterBy;
public:
	const QString uiName;
	
private:
	bool enabled;
	bool inverted;
	
	static const QString savedFiltersAppliedSeparator;
	static const QString savedFiltersAppliedYesString;
	static const QString savedFiltersAppliedNoString;
	
protected:
	Filter(DataType type, const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
public:
	virtual ~Filter();
	
public:
	bool isEnabled() const;
	bool isInverted() const;
	void setEnabled(bool enabled);
	void setInverted(bool inverted);
	
	void applyToOrderBuffer(ViewOrderBuffer& viewOrderBuffer) const;
protected:
	virtual bool evaluate(const QVariant& rawRowValue) const = 0;
	
public:
	virtual FilterBox* createFilterBox(QWidget* parent) = 0;
	
	
	// Encoding & Decoding
	static QString encodeToString(QList<const Filter*> filters, bool filtersApplied);
private:
	QString encodeSingleFilterToString() const;
public:
	static QList<Filter*> decodeFromString(const QString& encoded, const ItemTypesHandler& typesHandler, bool* const filtersAppliedResult);
private:
	static Filter* decodeSingleFilterFromString(QString& restOfEncoding, const ItemTypesHandler& typesHandler);
protected:
	virtual QStringList encodeTypeSpecific() const = 0;
};



#endif // FILTER_H
