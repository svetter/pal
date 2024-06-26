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

#ifndef ENUM_FILTER_BOX_H
#define ENUM_FILTER_BOX_H

#include "src/filters/enum_filter.h"
#include "src/filters/filter_widgets/filter_box.h"

#include <QComboBox>



class EnumFilterBox : public FilterBox
{
	Q_OBJECT
	
	EnumFilter& filter;
	
	QComboBox* combo;
	const QStringList entries;
	
public:
	explicit EnumFilterBox(QWidget* parent, const QString& title, const QStringList& entries, EnumFilter& filter);
	virtual ~EnumFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
};



#endif // ENUM_FILTER_BOX_H
