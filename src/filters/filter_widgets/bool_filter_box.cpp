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

#include "bool_filter_box.h"



BoolFilterBox::BoolFilterBox(QWidget* parent, const QString& title, BoolFilter& filter) :
	FilterBox(parent, Bit, title, filter),
	filter(filter),
	yesRadiobutton(new QRadioButton(this)),
	noRadiobutton(new QRadioButton(this)),
	yesNoButtonGroup(QButtonGroup()),
	spacerL(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored)),
	spacerR(new QSpacerItem(QSizePolicy::Expanding, QSizePolicy::Ignored))
{
	yesRadiobutton->setObjectName("yesRadiobutton");
	yesRadiobutton->setText(tr("Yes"));
	yesRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	noRadiobutton->setObjectName("noRadiobutton");
	noRadiobutton->setText(tr("No"));
	noRadiobutton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	yesNoButtonGroup.addButton(yesRadiobutton);
	yesNoButtonGroup.addButton(noRadiobutton);
	
	filterLayout->addItem(spacerL);
	filterLayout->addWidget(yesRadiobutton);
	filterLayout->addWidget(noRadiobutton);
	filterLayout->addItem(spacerR);
	
	connect(&yesNoButtonGroup,	&QButtonGroup::buttonClicked,	this,	&BoolFilterBox::updateFilterTypeSpecific);
	
	yesRadiobutton->setChecked(filter.value);
	noRadiobutton->setChecked(!filter.value);
}

BoolFilterBox::~BoolFilterBox()
{
	// Widgets deleted by layout
}



void BoolFilterBox::updateFilterTypeSpecific()
{
	filter.value = !noRadiobutton->isChecked();
	emit filterChanged();
}



const Filter& BoolFilterBox::getFilter() const
{
	return filter;
}
