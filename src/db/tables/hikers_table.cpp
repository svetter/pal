/*
 * Copyright 2023 Simon Vetter
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
 * @file src/db/tables/hikers_table.h
 * 
 * This file defines the HikersTable class.
 */

#include "hikers_table.h"

#include <QString>
#include <QTranslator>



/**
 * Creates a new HikersTable.
 */
HikersTable::HikersTable() :
		NormalTable(QString("Hikers"), tr("Hikers"), "hikerID"),
		//						name	uiName		type	nullable	primaryKey	foreignKey	inTable
		nameColumn	(new Column("name",	tr("Name"),	String,	false,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
}



/**
 * Adds a new hiker to the table.
 *
 * @param parent	The parent widget.
 * @param hiker	The hiker to add.
 * @return			The index of the new hiker in the table buffer.
 */
BufferRowIndex HikersTable::addRow(QWidget* parent, Hiker* hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, hiker);
	
	BufferRowIndex newHikerIndex = NormalTable::addRow(parent, columns, data);
	hiker->hikerID = getPrimaryKeyAt(newHikerIndex);
	return newHikerIndex;
}

/**
 * Updates the contents of an existing hiker in the table.
 *
 * @pre The given hiker's ItemID will be used to identify the hiker to update and thus must be valid.
 *
 * @param parent	The parent widget.
 * @param hiker		The hiker to update.
 */
void HikersTable::updateRow(QWidget* parent, ValidItemID hikerID, const Hiker* hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, hiker);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


/**
 * Translates the data of a hiker to a list of QVariants.
 *
 * @param columns	The column list specifying the order of the data.
 * @param hiker		The hiker from which to get the data.
 * @return			The list of QVariants representing the hiker's data.
 */
QList<QVariant> HikersTable::mapDataToQVariantList(QList<const Column*>& columns, const Hiker* hiker) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)	{ data.append(hiker->name);	continue; }
		assert(false);
	}
	return data;
}



/**
 * Returns the translated string to be displayed in a combo box containing hikers when none is
 * selected.
 *
 * @return	The translated string representing absence of a hiker.
 */
QString HikersTable::getNoneString() const
{
	return tr("None");
}

/**
 * Returns the translation of "hiker" (singular) in lowercase.
 *
 * @return	The translation of "hiker" (singular) in lowercase.
 */
QString HikersTable::getItemNameSingularLowercase() const
{
	return tr("hiker");
}

/**
 * Returns the translation of "hikers" (plural) in lowercase.
 *
 * @return	The translation of "hikers" (plural) in lowercase.
 */
QString HikersTable::getItemNamePluralLowercase() const
{
	return tr("hikers");
}
