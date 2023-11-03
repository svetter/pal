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
 * @file peaks_table.h
 * 
 * This file declares the PeaksTable class.
 */

#ifndef PEAKS_TABLE_H
#define PEAKS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/peak.h"

#include <QWidget>



/**
 * A class for accessing and manipulating the ascents table in the database.
 */
class PeaksTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The name column. */
	Column* const nameColumn;
	/** The height column. */
	Column* const heightColumn;
	/** The volcano column. */
	Column* const volcanoColumn;
	/** The region ID column. */
	Column* const regionIDColumn;
	/** The Google Maps link column. */
	Column* const mapsLinkColumn;
	/** The Google Earth link column. */
	Column* const earthLinkColumn;
	/** The Wikipedia link column. */
	Column* const wikiLinkColumn;
	
	PeaksTable(Column* foreignRegionIDColumn);
	
	BufferRowIndex addRow(QWidget* parent, Peak* peak);
	void updateRow(QWidget* parent, ValidItemID peakID, const Peak* peak);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Peak* peak) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // PEAKS_TABLE_H
