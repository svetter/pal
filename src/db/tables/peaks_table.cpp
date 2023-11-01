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

#include "peaks_table.h"

#include <QString>
#include <QTranslator>



PeaksTable::PeaksTable(Column* foreignRegionIDColumn) :
		NormalTable(QString("Peaks"), tr("Peaks"), "peakID"),
		//							name			uiName						type		nullable	primaryKey	foreignKey				inTable
		nameColumn		(new Column("name",			tr("Name"),					String,		false,		false,		nullptr,				this)),
		heightColumn	(new Column("height",		tr("Height"),				Integer,	true,		false,		nullptr,				this)),
		volcanoColumn	(new Column("volcano",		tr("Volcano"),				Bit,		false,		false,		nullptr,				this)),
		regionIDColumn	(new Column("regionID",		QString(),					ID,			true,		false,		foreignRegionIDColumn,	this)),
		mapsLinkColumn	(new Column("mapsLink",		tr("Google Maps link"),		String,		true,		false,		nullptr,				this)),
		earthLinkColumn	(new Column("earthLink",	tr("Google Earth link"),	String,		true,		false,		nullptr,				this)),
		wikiLinkColumn	(new Column("wikiLink",		tr("Wikipedia link"),		String,		true,		false,		nullptr,				this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
	addColumn(heightColumn);
	addColumn(volcanoColumn);
	addColumn(regionIDColumn);
	addColumn(mapsLinkColumn);
	addColumn(earthLinkColumn);
	addColumn(wikiLinkColumn);
}



BufferRowIndex PeaksTable::addRow(QWidget* parent, Peak* peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, peak);
	
	BufferRowIndex newPeakIndex = NormalTable::addRow(parent, columns, data);
	peak->peakID = getPrimaryKeyAt(newPeakIndex);
	return newPeakIndex;
}

void PeaksTable::updateRow(QWidget* parent, ValidItemID peakID, const Peak* peak)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, peak);
	
	NormalTable::updateRow(parent, peakID, columns, data);
}


QList<QVariant> PeaksTable::mapDataToQVariantList(QList<const Column*>& columns, const Peak* peak) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)		{ data.append(peak->name);					continue; }
		if (column == heightColumn)		{ data.append(peak->getHeightAsQVariant());	continue; }
		if (column == volcanoColumn)	{ data.append(peak->volcano);				continue; }
		if (column == regionIDColumn)	{ data.append(peak->regionID.asQVariant());	continue; }
		if (column == mapsLinkColumn)	{ data.append(peak->mapsLink);				continue; }
		if (column == earthLinkColumn)	{ data.append(peak->earthLink);				continue; }
		if (column == wikiLinkColumn)	{ data.append(peak->wikiLink);				continue; }
		assert(false);
	}
	return data;
}



QString PeaksTable::getNoneString() const
{
	return tr("None");
}

QString PeaksTable::getItemNameSingularLowercase() const
{
	return tr("peak");
}

QString PeaksTable::getItemNamePluralLowercase() const
{
	return tr("peaks");
}
