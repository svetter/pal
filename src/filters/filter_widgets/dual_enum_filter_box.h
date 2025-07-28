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

#ifndef DUAL_ENUM_FILTER_BOX_H
#define DUAL_ENUM_FILTER_BOX_H

#include "src/filters/dual_enum_filter.h"
#include "src/filters/filter_widgets/filter_box.h"

#include <QComboBox>



class DualEnumFilterBox : public FilterBox
{
	Q_OBJECT
	
	DualEnumFilter& filter;
	
	QComboBox* comboDiscerning;
	QComboBox* comboDependent;
	const QList<QPair<QString, QStringList>> entries;
	
public:
	explicit DualEnumFilterBox(QWidget* parent, const QString& title, const QList<QPair<QString, QStringList>>& entries, DualEnumFilter& filter);
	virtual ~DualEnumFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
	
private slots:
	void handle_discerningComboChanged();
};



#endif // DUAL_ENUM_FILTER_BOX_H
