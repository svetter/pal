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

#include "hikers_table.h"

#include <QString>
#include <QTranslator>



HikersTable::HikersTable() :
		NormalTable(QString("Hikers"), tr("Hikers"), "hikerID"),
		//						name	uiName		type	nullable	primaryKey	foreignKey	inTable
		nameColumn	(new Column("name",	tr("Name"),	String,	false,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
}



int HikersTable::addRow(QWidget* parent, Hiker* hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, hiker);
	
	int newHikerIndex = NormalTable::addRow(parent, columns, data);
	hiker->hikerID = getPrimaryKeyAt(newHikerIndex);
	return newHikerIndex;
}

void HikersTable::updateRow(QWidget* parent, ValidItemID hikerID, const Hiker* hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, hiker);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> HikersTable::mapDataToQVariantList(QList<const Column*>& columns, const Hiker* hiker) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)	{ data.append(hiker->name);	continue; }
		assert(false);
	}
	return data;
}



QString HikersTable::getNoneString() const
{
	return tr("None");
}

QString HikersTable::getItemNameSingularLowercase() const
{
	return tr("hiker");
}

QString HikersTable::getItemNamePluralLowercase() const
{
	return tr("hikers");
}
