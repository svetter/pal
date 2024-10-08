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

#include "date_filter_box.h"



DateFilterBox::DateFilterBox(QWidget* parent, const QString& title, DateFilter& filter) :
	FilterBox(parent, Date, title, filter),
	filter(filter),
	minDateWidget(new QDateEdit(this)),
	maxDateWidget(new QDateEdit(this)),
	setMaxDateCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	minDateWidget->setObjectName("minDateWidget");
	minDateWidget->setMinimumSize(QSize(80, 0));
	minDateWidget->setCalendarPopup(true);
	
	setMaxDateCheckbox->setObjectName("setMaxDateCheckbox");
	setMaxDateCheckbox->setToolTip(tr("Set a maximum date"));
	setMaxDateCheckbox->setChecked(true);
	
	maxDateWidget->setObjectName("maxDateWidget");
	maxDateWidget->setMinimumSize(QSize(80, 0));
	maxDateWidget->setCalendarPopup(true);
	
	filterLayout->addWidget(minDateWidget);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxDateCheckbox);
	filterLayout->addWidget(maxDateWidget);
	
	connect(minDateWidget,		&QDateEdit::dateChanged,		this,	&DateFilterBox::handle_minDateChanged);
	connect(setMaxDateCheckbox,	&QCheckBox::checkStateChanged,	this,	&DateFilterBox::handle_setMaxDateChanged);
	connect(maxDateWidget,		&QDateEdit::dateChanged,		this,	&DateFilterBox::handle_maxDateChanged);
	
	minDateWidget->setDate(filter.min);
	maxDateWidget->setDate(filter.max);
	setMaxDateCheckbox->setChecked(filter.min < filter.max);
}

DateFilterBox::~DateFilterBox()
{
	// Widgets deleted by layout
}



void DateFilterBox::updateFilterTypeSpecific()
{
	filter.min = minDateWidget->date();
	filter.max = maxDateWidget->date();
	emit filterChanged();
}



const Filter& DateFilterBox::getFilter() const
{
	return filter;
}



void DateFilterBox::handle_minDateChanged()
{
	if (!setMaxDateCheckbox->isChecked()) {
		maxDateWidget->setDate(minDateWidget->date());
	}
	else if (minDateWidget->date() > maxDateWidget->date()) {
		maxDateWidget->setDate(minDateWidget->date());
	}
	
	updateFilterTypeSpecific();
}

void DateFilterBox::handle_maxDateChanged()
{
	if (minDateWidget->date() > maxDateWidget->date()) {
		return minDateWidget->setDate(maxDateWidget->date());
	}
	
	updateFilterTypeSpecific();
}

void DateFilterBox::handle_setMaxDateChanged()
{
	maxDateWidget->setEnabled(setMaxDateCheckbox->isChecked());
	handle_minDateChanged();
}
