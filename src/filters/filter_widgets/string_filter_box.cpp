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

#include "string_filter_box.h"



StringFilterBox::StringFilterBox(QWidget* parent, const QString& title, StringFilter& filter) :
	FilterBox(parent, String, title, filter),
	filter(filter),
	lineEdit(new QLineEdit(this))
{
	lineEdit->setObjectName("lineEdit");
	lineEdit->setMinimumWidth(150);
	lineEdit->setPlaceholderText(tr("Contains..."));
	
	filterLayout->addWidget(lineEdit);
	
	connect(lineEdit,	&QLineEdit::textChanged,	this,	&StringFilterBox::updateFilterTypeSpecific);
	
	lineEdit->setText(filter.value);
}

StringFilterBox::~StringFilterBox()
{
	// Widgets deleted by layout
}



void StringFilterBox::updateFilterTypeSpecific()
{
	filter.value = lineEdit->text();
	emit filterChanged();
}



const Filter& StringFilterBox::getFilter() const
{
	return filter;
}
