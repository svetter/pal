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
 * This file declares the CountriesTable class.
 */

#ifndef COUNTRIES_TABLE_H
#define COUNTRIES_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/country.h"

#include <QWidget>



/**
 * A class for accessing and manipulating the ascents table in the database.
 */
class CountriesTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The name column. */
	ValueColumn nameColumn;
	
	CountriesTable(Database& db);
	
	BufferRowIndex addRow(QWidget& parent, Country& country);
	void updateRow(QWidget& parent, ValidItemID countryID, const Country& country);
	void updateRows(QWidget& parent, const QSet<BufferRowIndex>& rowIndices, const QList<const Column*> columns, const Country& country);
private:
	const QList<ColumnDataPair> mapDataToColumnDataPairs(const QList<const Column*>& columns, const Country& country) const;
	
public:
	virtual QString getIdentityRepresentationAt(const BufferRowIndex& bufferRow) const override;
	virtual QList<const Column*> getIdentityRepresentationColumns() const override;
	
	virtual QString getItemNameSingular() const override;
	virtual QString getNoneString() const override;
	virtual QString getEditItemString() const override;
	virtual QString getCreationConfirmMessage() const override;
	virtual QString getEditConfirmMessage(int numEdited) const override;
	virtual QString getDeleteConfirmMessage(int numDeleted) const override;
	virtual QString getNewCustomColumnString() const override;
	virtual QString getNewFilterString() const override;
	virtual QString getItemCountString(int numItems) const override;
};



#endif // COUNTRIES_TABLE_H
