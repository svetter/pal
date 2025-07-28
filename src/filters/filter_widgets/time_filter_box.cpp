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

#include "time_filter_box.h"



TimeFilterBox::TimeFilterBox(QWidget* parent, const QString& title, TimeFilter& filter) :
	FilterBox(parent, Time, title, filter),
	filter(filter),
	minTimeWidget(new QTimeEdit(this)),
	maxTimeWidget(new QTimeEdit(this)),
	setMaxTimeCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	minTimeWidget->setObjectName("minTimeWidget");
	
	setMaxTimeCheckbox->setObjectName("setMaxTimeCheckbox");
	setMaxTimeCheckbox->setToolTip(tr("Set a maximum time"));
	setMaxTimeCheckbox->setChecked(true);
	
	maxTimeWidget->setObjectName("maxTimeWidget");
	
	filterLayout->addWidget(minTimeWidget);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxTimeCheckbox);
	filterLayout->addWidget(maxTimeWidget);
	
	connect(minTimeWidget,		&QTimeEdit::timeChanged,		this,	&TimeFilterBox::handle_minTimeChanged);
	connect(setMaxTimeCheckbox,	&QCheckBox::checkStateChanged,	this,	&TimeFilterBox::handle_setMaxTimeChanged);
	connect(maxTimeWidget,		&QTimeEdit::timeChanged,		this,	&TimeFilterBox::handle_maxTimeChanged);
	
	minTimeWidget->setTime(filter.min);
	maxTimeWidget->setTime(filter.max);
	setMaxTimeCheckbox->setChecked(filter.min < filter.max);
}

TimeFilterBox::~TimeFilterBox()
{
	// Widgets deleted by layout
}



void TimeFilterBox::updateFilterTypeSpecific()
{
	filter.min = minTimeWidget->time();
	filter.max = maxTimeWidget->time();
	emit filterChanged();
}



const Filter& TimeFilterBox::getFilter() const
{
	return filter;
}



void TimeFilterBox::handle_minTimeChanged()
{
	if (!setMaxTimeCheckbox->isChecked()) {
		maxTimeWidget->setTime(minTimeWidget->time());
	}
	else if (minTimeWidget->time() > maxTimeWidget->time()) {
		maxTimeWidget->setTime(minTimeWidget->time());
	}
	
	updateFilterTypeSpecific();
}

void TimeFilterBox::handle_maxTimeChanged()
{
	if (minTimeWidget->time() > maxTimeWidget->time()) {
		return minTimeWidget->setTime(maxTimeWidget->time());
	}
	
	updateFilterTypeSpecific();
}

void TimeFilterBox::handle_setMaxTimeChanged()
{
	maxTimeWidget->setEnabled(setMaxTimeCheckbox->isChecked());
	handle_minTimeChanged();
}
