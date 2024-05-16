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

#ifndef INT_FILTER_BOX_H
#define INT_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/int_filter.h"

#include <QSpinBox>
#include <QCheckBox>



class IntFilterBox : public FilterBox
{
	Q_OBJECT
	
	IntFilter& filter;
	
	QSpinBox* minSpinner;
	QSpinBox* maxSpinner;
	QCheckBox* setMaxCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit IntFilterBox(QWidget* parent, const QString& title, IntFilter& filter);
	virtual ~IntFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
	
private slots:
	void handle_minChanged();
	void handle_maxChanged();
	void handle_setMaxChanged();
};



#endif // INT_FILTER_BOX_H
