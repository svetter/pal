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

#ifndef BOOL_FILTER_BOX_H
#define BOOL_FILTER_BOX_H

#include "src/filters/bool_filter.h"
#include "src/filters/filter_widgets/filter_box.h"

#include <QCheckBox>



class BoolFilterBox : public FilterBox
{
	Q_OBJECT
	
	BoolFilter& filter;
	
	QRadioButton* yesRadiobutton;
	QRadioButton* noRadiobutton;
	QButtonGroup yesNoButtonGroup;
	QSpacerItem* spacerL;
	QSpacerItem* spacerR;
	
public:
	explicit BoolFilterBox(QWidget* parent, const QString& title, BoolFilter& filter);
	virtual ~BoolFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
};



#endif // BOOL_FILTER_BOX_H
