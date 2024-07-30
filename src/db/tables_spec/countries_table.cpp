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
 * 
 * @param db	The database to which the table belongs.
 */
CountriesTable::CountriesTable(Database& db) :
	NormalTable(db, QString("Countries"), tr("Countries"), "countryID", tr("Country ID")),
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
BufferRowIndex CountriesTable::addRow(QWidget& parent, Country& country)
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
 * @param parent	The parent widget.
 * @param countryID	The ID of the country to update.
 * @param country	The country data to write to the row with the given ID.
 */
void CountriesTable::updateRow(QWidget& parent, ValidItemID countryID, const Country& country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, country);
	
	NormalTable::updateRow(parent, countryID, columnDataPairs);
}

/**
 * Updates the contents of existing countries in the table.
 * 
 * @param parent		The parent widget.
 * @param rowIndices	The indices of the countries to update in the table buffer.
 * @param columns		The columns to update. Can not contain the primary key column.
 * @param country		The new data for the countries. The countryID will be ignored.
 */
void CountriesTable::updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Country& country)
{
	assert(!columns.contains(&primaryKeyColumn));
	const QList<ColumnDataPair> columnDataPairs = mapDataToColumnDataPairs(columns, country);
	
	NormalTable::updateRows(parent, rowIndices, columnDataPairs);
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
		
		if (column->type == String && data.toString().isEmpty()) data = QVariant();
		
		columnDataPairs.append({column, data});
	}
	return columnDataPairs;
}



/**
 * Returns a string representation of the country at the given buffer row index.
 * 
 * @param bufferRow	The buffer row index of the country to represent.
 * @return			A UI-appropriate string representation of the country.
 */
QString CountriesTable::getIdentityRepresentationAt(const BufferRowIndex& bufferRow) const
{
	return nameColumn.getValueAt(bufferRow).toString();
}

/**
 * Returns a list of all columns used for identity representation of countries.
 * 
 * @return	A list of all columns used for identity representation.
 */
QList<const Column*> CountriesTable::getIdentityRepresentationColumns() const
{
	return { &nameColumn };
}



/**
 * Returns the translation of "Country" (singular).
 * 
 * @return	The translation of "Country" (singular).
 */
QString CountriesTable::getItemNameSingular() const
{
	return tr("Country");
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
 * Returns a translated prompt to edit an existing country.
 * 
 * @return	The translated prompt to edit an existing country.
 */
QString CountriesTable::getEditItemString() const
{
	return tr("Edit country...");
}

/**
 * Returns a translated message confirming that a new country has been created.
 * 
 * @return	The translated message confirming creation of a new country.
 */
QString CountriesTable::getCreationConfirmMessage() const
{
	return tr("Saved new country.");
}

/**
 * Returns a translated message confirming that a number of countries have been edited.
 * 
 * @param numEdited	The number of countries that have been edited.
 * @return			The translated message confirming the editing of the countries.
 */
QString CountriesTable::getEditConfirmMessage(int numEdited) const
{
	return tr("Saved changes in %Ln country/countries.", "", numEdited);
}

/**
 * Returns a translated message confirming that a number of countries have been deleted.
 * 
 * @param numDeleted	The number of countries that have been deleted.
 * @return				The translated message confirming the deletion of the countries.
 */
QString CountriesTable::getDeleteConfirmMessage(int numDeleted) const
{
	return tr("Deleted %Ln country/countries.", "", numDeleted);
}

/**
 * Returns a translated title for a country custom column wizard.
 * 
 * @return	The translated title for a country custom column wizard.
 */
QString CountriesTable::getNewCustomColumnString() const
{
	return tr("New custom column for countries");
}

/**
 * Returns a translated title for a country filter wizard.
 * 
 * @return	The translated title for a country filter wizard.
 */
QString CountriesTable::getNewFilterString() const
{
	return tr("New country filter");
}

/**
 * Returns a translated string which can be used in lists of items and their counts, in the form
 * "n country/countries".
 * 
 * @param numItems	The number of countries to list.
 * @return			A translated list entry naming the item type and a corresponding count.
 */
QString CountriesTable::getItemCountString(int numItems) const
{
	return tr("%Ln country/countries", "", numItems);
}
