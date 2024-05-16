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

#ifndef TIME_FILTER_BOX_H
#define TIME_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/time_filter.h"

#include <QTimeEdit>
#include <QCheckBox>



class TimeFilterBox : public FilterBox
{
	Q_OBJECT
	
	TimeFilter& filter;
	
	QTimeEdit* minTimeWidget;
	QTimeEdit* maxTimeWidget;
	QCheckBox* setMaxTimeCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit TimeFilterBox(QWidget* parent, const QString& title, TimeFilter& filter);
	virtual ~TimeFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
	
private slots:
	void handle_minTimeChanged();
	void handle_maxTimeChanged();
	void handle_setMaxTimeChanged();
};



#endif // TIME_FILTER_BOX_H
