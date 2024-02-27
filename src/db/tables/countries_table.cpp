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

/**
 * @file countries_table.h
 * 
 * This file defines the CountriesTable class.
 */

#include "countries_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new CountriesTable.
 */
CountriesTable::CountriesTable() :
	NormalTable(QString("Countries"), tr("Countries"), "countryID", tr("Country ID")),
	//									name	uiName		type	nullable
	nameColumn	(ValueColumn	(*this,	"name",	tr("Name"),	String,	false))
{
	addColumn(nameColumn);
}



/**
 * Adds a new country to the table.
 *
 * @param parent	The parent widget.
 * @param country	The country to add.
 * @return			The index of the new country in the table buffer.
 */
BufferRowIndex CountriesTable::addRow(QWidget* parent, Country& country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, country);
	
	BufferRowIndex newCountryIndex = NormalTable::addRow(parent, columnDataPairs);
	country.countryID = getPrimaryKeyAt(newCountryIndex);
	return newCountryIndex;
}

/**
 * Updates the contents of an existing country in the table.
 *
 * @pre The given country's ItemID will be used to identify the country to update and thus must be valid.
 *
 * @param parent	The parent widget.
 * @param country	The country to update.
 */
void CountriesTable::updateRow(QWidget* parent, ValidItemID countryID, const Country& country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, country);
	
	NormalTable::updateRow(parent, countryID, columnDataPairs);
}


/**
 * Translates the data of a country to a list of column-data pairs.
 *
 * @param columns	The column list specifying the order of the data.
 * @param country	The country from which to get the data.
 * @return			A list of column-data pairs representing the country's data.
 */
const QList<ColumnDataPair> CountriesTable::mapDataToColumnDataPairs(const QList<const Column*>& columns, const Country& country) const
{
	QList<ColumnDataPair> columnDataPairs = QList<ColumnDataPair>();
	for (const Column* const column : columns) {
		QVariant data;
		     if (column == &nameColumn)	{ data = country.name;	}
		else assert(false);
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns the translated string to be displayed to indicate that no country is selected.
 *
 * @return	The translated string representing absence of a country.
 */
QString CountriesTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns the translation of "country" (singular), not capitalized unless the language requires
 * it.
 *
 * @return	The the translation of "country" (singular) for use mid-sentence.
 */
QString CountriesTable::getItemNameSingularLowercase() const
{
	return tr("country");
}

/**
 * Returns the translation of "countries" (plural), not capitalized unless the language requires
 * it.
 *
 * @return	The translation of "countries" (plural) for use mid-sentence.
 */
QString CountriesTable::getItemNamePluralLowercase() const
{
	return tr("countries");
}
