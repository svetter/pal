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

#include "id_filter.h"

#include "src/comp_tables/fold_composite_column.h"
#include "src/db/database.h"
#include "src/dialogs/item_dialog.h"
#include "src/filters/filter_widgets/id_filter_box.h"

#include <QComboBox>

using std::placeholders::_1, std::placeholders::_2;



IDFilter::IDFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(ID, tableToFilter, columnToFilterBy, uiName),
	value(ItemID())
{}

IDFilter::~IDFilter()
{}



bool IDFilter::evaluate(const QVariant& rawRowValue) const
{
	/*                              ╔═══════════════════════════════════════════════╗
	 *                              ║           Value from filtered table           ║
	 * ╔════════════════════════════╬───────────────┬────────────────┬──────────────╢
	 * ║        Filter              ║ QVariant null │ Invalid ItemID │ Valid ItemID ║
	 * ╟───────────┬────────────────╬═══════════════╪════════════════╪══════════════╣
	 * ║  Include  │ Invalid ItemID ║ true          │ true           │ false        ║
	 * ║    i.e.   ├────────────────╫───────────────┼────────────────┼──────────────╢
	 * ║ !inverted │   Valid ItemID ║ false         │ false          │ match        ║
	 * ╟───────────┼────────────────╫───────────────┼────────────────┼──────────────╢
	 * ║  Exclude  │ Invalid ItemID ║ false         │ false          │ true         ║
	 * ║    i.e.   ├────────────────╫───────────────┼────────────────┼──────────────╢
	 * ║  inverted │   Valid ItemID ║ true          │ true           │ !match       ║
	 * ╚═══════════╧════════════════╩═══════════════╧════════════════╧══════════════╝
	 */
	
	if (rawRowValue.isNull()) {
		return value.isValid() == isInverted();
	}
	
	if (rawRowValue.canConvert<int>()) {
		const ItemID convertedValue = ItemID(rawRowValue.toInt());
		bool match;
		if (convertedValue.isValid()) {
			match = convertedValue == value;
		} else {
			match = value.isInvalid();
		}
		return match != isInverted();
	}
	
	if (rawRowValue.canConvert<QVariantList>()) {
		const QVariantList list = rawRowValue.toList();
		bool match;
		if (value.isValid()) {
			match = list.contains(value.asQVariant());
		} else {
			match = list.isEmpty();
		}
		return match != isInverted();
	}
	
	assert(false);
	return false;
}



FilterBox* IDFilter::createFilterBox(QWidget* parent)
{
	const bool columnProxy = columnToFilterBy.contentType != ID;
	const Column* contentColumn = nullptr;
	switch (columnToFilterBy.type) {
	case Direct:			contentColumn = &((DirectCompositeColumn&)			columnToFilterBy).contentColumn;	break;
	case Reference:			contentColumn = &((ReferenceCompositeColumn&)		columnToFilterBy).contentColumn;	break;
	case ListStringFold:
	case HikerListFold:		contentColumn = ((ListStringFoldCompositeColumn&)	columnToFilterBy).contentColumn;	break;
	default: assert(false);
	}
	assert(contentColumn);
	if (columnProxy) {
		assert(!contentColumn->table.isAssociative);
		contentColumn = &((NormalTable&) contentColumn->table).primaryKeyColumn;
	}
	assert(contentColumn->primaryKey);
	NormalTable* idTable = (NormalTable*) &contentColumn->table;
	Database& db = idTable->db;
	
	std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo;
	     if (idTable == &db.ascentsTable)	populateItemCombo = std::bind(populateAscentCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.peaksTable)		populateItemCombo = std::bind(populatePeakCombo,	std::ref(db), _1, _2,	ItemID());
	else if (idTable == &db.tripsTable)		populateItemCombo = std::bind(populateTripCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.hikersTable)	populateItemCombo = std::bind(populateHikerCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.regionsTable)	populateItemCombo = std::bind(populateRegionCombo,	std::ref(db), _1, _2,	false);
	else if (idTable == &db.rangesTable)	populateItemCombo = std::bind(populateRangeCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.countriesTable)	populateItemCombo = std::bind(populateCountryCombo,	std::ref(db), _1, _2);
	else assert(false);
	
	return new IDFilterBox(parent, uiName, populateItemCombo, *this);
}



QStringList IDFilter::encodeTypeSpecific() const
{
	return {
		encodeID("value", value)
	};
}

IDFilter* IDFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const ItemID value = decodeID(restOfEncoding, "value", ok);
	if (!ok) return nullptr;
	
	IDFilter* const filter = new IDFilter(tableToFilter, columnToFilterBy, uiName);
	filter->value = value;
	
	return filter;
}
