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

#include "enum_filter_box.h"

#include "src/data/enum_names.h"

#include <QAbstractItemView>



EnumFilterBox::EnumFilterBox(QWidget* parent, const QString& title, const QStringList& entries, EnumFilter& filter) :
	FilterBox(parent, Enum, title, filter),
	filter(filter),
	combo(new QComboBox(this)),
	entries(entries)
{
	combo->setObjectName("combo");
	combo->insertItems(0, EnumNames::translateList(entries));
	combo->setMaximumWidth(200);
	combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	combo->view()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	
	filterLayout->addWidget(combo);
	
	connect(combo,	&QComboBox::currentIndexChanged,	this,	&EnumFilterBox::updateFilterTypeSpecific);
	
	combo->setCurrentIndex(filter.value);
}

EnumFilterBox::~EnumFilterBox()
{
	// Widgets deleted by layout
}



void EnumFilterBox::updateFilterTypeSpecific()
{
	filter.value = combo->currentIndex();
	emit filterChanged();
}



const Filter& EnumFilterBox::getFilter() const
{
	return filter;
}
