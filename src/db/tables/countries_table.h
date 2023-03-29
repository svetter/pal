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

#ifndef COUNTRIES_TABLE_H
#define COUNTRIES_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/country.h"

#include <QWidget>



class CountriesTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* const nameColumn;
	
	CountriesTable();
	
	int addRow(QWidget* parent, Country* country);
	void updateRow(QWidget* parent, ValidItemID countryID, const Country* country);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Country* country) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // COUNTRIES_TABLE_H
