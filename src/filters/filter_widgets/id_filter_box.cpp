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

#include "id_filter_box.h"

#include "src/data/item_id.h"

#include <QAbstractItemView>



IDFilterBox::IDFilterBox(QWidget* parent, const QString& title, std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo, IDFilter& filter) :
	FilterBox(parent, ID, title, filter),
	filter(filter),
	combo(new QComboBox(this)),
	populateItemCombo(populateItemCombo),
	selectableItemIDs(QList<ValidItemID>())
{
	combo->setObjectName("combo");
	combo->setMaximumWidth(200);
	combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	combo->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
	combo->view()->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	
	filterLayout->addWidget(combo);
	
	setComboEntries();
	
	connect(combo,	&QComboBox::currentIndexChanged,	this,	&IDFilterBox::updateFilterTypeSpecific);
	
	for (int i = 0; i < selectableItemIDs.size(); i++) {
		if (selectableItemIDs.at(i) == filter.value) {
			combo->setCurrentIndex(i + 1);	// 0 is None
			break;
		}
	}
}

IDFilterBox::~IDFilterBox()
{
	// Widgets deleted by layout
}



void IDFilterBox::setComboEntries()
{
	ItemID previouslySelectedItemID = ItemID();
	const int comboIndex = combo->currentIndex();
	if (comboIndex > 0) {
		previouslySelectedItemID = selectableItemIDs.at(comboIndex - 1);	// 0 is None
	}
	
	populateItemCombo(*combo, selectableItemIDs);
	
	int newComboIndex = 0;
	ItemID newlySelectedItemID = ItemID();
	if (selectableItemIDs.contains(previouslySelectedItemID)) {
		newComboIndex = selectableItemIDs.indexOf(previouslySelectedItemID) + 1;	// 0 is None
		newlySelectedItemID = previouslySelectedItemID;
	}
	combo->setCurrentIndex(newComboIndex);
	
	if (newlySelectedItemID != previouslySelectedItemID) {
		emit filterChanged();
	}
}



void IDFilterBox::updateFilterTypeSpecific()
{
	const int comboIndex = combo->currentIndex();
	if (comboIndex < 1) {
		filter.value = ItemID();
	}
	else {
		filter.value = selectableItemIDs.at(combo->currentIndex() - 1);	// 0 is None
	}
	
	emit filterChanged();
}



const Filter& IDFilterBox::getFilter() const
{
	return filter;
}
