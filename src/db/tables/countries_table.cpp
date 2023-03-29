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

#include "countries_table.h"

#include <QString>
#include <QTranslator>



CountriesTable::CountriesTable() :
		NormalTable(QString("Countries"), tr("Countries"), "countryID"),
		//						name	uiName		type	nullable	primaryKey	foreignKey	inTable
		nameColumn	(new Column("name",	tr("Name"),	String,	false,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
}



int CountriesTable::addRow(QWidget* parent, Country* country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, country);
	
	int newCountryIndex = NormalTable::addRow(parent, columns, data);
	country->countryID = getPrimaryKeyAt(newCountryIndex);
	return newCountryIndex;
}

void CountriesTable::updateRow(QWidget* parent, ValidItemID hikerID, const Country* country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, country);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> CountriesTable::mapDataToQVariantList(QList<const Column*>& columns, const Country* country) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)	{ data.append(country->name);	continue; }
		assert(false);
	}
	return data;
}



QString CountriesTable::getNoneString() const
{
	return tr("None");
}

QString CountriesTable::getItemNameSingularLowercase() const
{
	return tr("country");
}

QString CountriesTable::getItemNamePluralLowercase() const
{
	return tr("countries");
}
