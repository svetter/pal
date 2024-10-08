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

#include "int_filter_box.h"



IntFilterBox::IntFilterBox(QWidget* parent, const QString& title, IntFilter& filter) :
	FilterBox(parent, Integer, title, filter),
	filter(filter),
	minSpinner(new QSpinBox(this)),
	maxSpinner(new QSpinBox(this)),
	setMaxCheckbox(new QCheckBox(this)),
	spacer(new QSpacerItem(5, 0, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
	minSpinner->setObjectName("minSpinner");
	minSpinner->setMaximum(9999999);
	
	setMaxCheckbox->setObjectName("setMaxCheckbox");
	setMaxCheckbox->setToolTip(tr("Set a maximum value"));
	setMaxCheckbox->setChecked(true);
	
	maxSpinner->setObjectName("maxSpinner");
	maxSpinner->setMaximum(9999999);
	
	filterLayout->addWidget(minSpinner);
	filterLayout->addItem(spacer);
	filterLayout->addWidget(setMaxCheckbox);
	filterLayout->addWidget(maxSpinner);
	
	connect(minSpinner,		&QSpinBox::valueChanged,		this,	&IntFilterBox::handle_minChanged);
	connect(setMaxCheckbox,	&QCheckBox::checkStateChanged,	this,	&IntFilterBox::handle_setMaxChanged);
	connect(maxSpinner,		&QSpinBox::valueChanged,		this,	&IntFilterBox::handle_maxChanged);
	
	minSpinner->setValue(filter.min);
	maxSpinner->setValue(filter.max);
	setMaxCheckbox->setChecked(filter.min < filter.max);
}

IntFilterBox::~IntFilterBox()
{
	// Widgets deleted by layout
}



void IntFilterBox::updateFilterTypeSpecific()
{
	filter.min = minSpinner->value();
	filter.max = maxSpinner->value();
	emit filterChanged();
}



const Filter& IntFilterBox::getFilter() const
{
	return filter;
}



void IntFilterBox::handle_minChanged()
{
	if (!setMaxCheckbox->isChecked()) {
		maxSpinner->setValue(minSpinner->value());
	}
	else if (minSpinner->value() > maxSpinner->value()) {
		maxSpinner->setValue(minSpinner->value());
	}
	
	updateFilterTypeSpecific();
}

void IntFilterBox::handle_maxChanged()
{
	if (minSpinner->value() > maxSpinner->value()) {
		return minSpinner->setValue(maxSpinner->value());
	}
	
	updateFilterTypeSpecific();
}

void IntFilterBox::handle_setMaxChanged()
{
	maxSpinner->setEnabled(setMaxCheckbox->isChecked());
	handle_minChanged();
}
