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

#ifndef DATE_FILTER_BOX_H
#define DATE_FILTER_BOX_H

#include "src/filters/date_filter.h"
#include "src/filters/filter_widgets/filter_box.h"

#include <QDateEdit>
#include <QCheckBox>



class DateFilterBox : public FilterBox
{
	Q_OBJECT
	
	DateFilter& filter;
	
	QDateEdit* minDateWidget;
	QDateEdit* maxDateWidget;
	QCheckBox* setMaxDateCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit DateFilterBox(QWidget* parent, const QString& title, DateFilter& filter);
	virtual ~DateFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
	
private slots:
	void handle_minDateChanged();
	void handle_maxDateChanged();
	void handle_setMaxDateChanged();
};



#endif // DATE_FILTER_BOX_H
