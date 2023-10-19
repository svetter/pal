/*
 * Copyright 2023 Simon Vetter
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

#include "region_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/main/settings.h"

#include <QMessageBox>



RegionDialog::RegionDialog(QWidget* parent, Database* db, DialogPurpose purpose, Region* init) :
		ItemDialog(parent, db, purpose),
		init(init),
		selectableRangeIDs(QList<ValidItemID>()),
		selectableCountryIDs(QList<ValidItemID>())
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/region_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::regionDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(newRangeButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_newRange);
	connect(newCountryButton,	&QPushButton::clicked,	this,	&RegionDialog::handle_newCountry);
	
	connect(okButton,			&QPushButton::clicked,	this,	&RegionDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	default:
		assert(false);
	}
}

RegionDialog::~RegionDialog()
{
	delete init;
}



QString RegionDialog::getEditWindowTitle()
{
	return tr("Edit region");
}



void RegionDialog::populateComboBoxes()
{
	populateItemCombo(db->rangesTable, db->rangesTable->nameColumn, true, rangeCombo, selectableRangeIDs);
	
	populateItemCombo(db->countriesTable, db->countriesTable->nameColumn, true, countryCombo, selectableCountryIDs);
}



void RegionDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Range
	if (init->rangeID.isValid()) {
		rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(init->rangeID.get()) + 1);	// 0 is None
	} else {
		rangeCombo->setCurrentIndex(0);
	}
	// Country
	if (init->countryID.isValid()) {
		countryCombo->setCurrentIndex(selectableCountryIDs.indexOf(init->countryID.get()) + 1);	// 0 is None
	} else {
		countryCombo->setCurrentIndex(0);
	}
}


Region* RegionDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	ItemID	rangeID		= parseItemCombo	(rangeCombo, selectableRangeIDs);
	ItemID	countryID	= parseItemCombo	(countryCombo, selectableCountryIDs);
	
	Region* region = new Region(ItemID(), name, rangeID, countryID);
	return region;
}


bool RegionDialog::changesMade()
{
	Region* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void RegionDialog::handle_newRange()
{
	int newRangeIndex = openNewRangeDialogAndStore(this, db);
	if (newRangeIndex < 0) return;
	
	populateItemCombo(db->rangesTable, db->rangesTable->nameColumn, true, rangeCombo, selectableRangeIDs);
	ValidItemID rangeID = db->rangesTable->getPrimaryKeyAt(newRangeIndex);
	rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(rangeID) + 1);	// 0 is None
}

void RegionDialog::handle_newCountry()
{
	int newCountryIndex = openNewCountryDialogAndStore(this, db);
	if (newCountryIndex < 0) return;
	
	populateItemCombo(db->countriesTable, db->countriesTable->nameColumn, true, countryCombo, selectableCountryIDs);
	ValidItemID countryID = db->countriesTable->getPrimaryKeyAt(newCountryIndex);
	countryCombo->setCurrentIndex(selectableCountryIDs.indexOf(countryID) + 1);	// 0 is None
}



void RegionDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save region");
	QString emptyNameWindowMessage	= tr("The region needs a name.");
	const Column* nameColumn = db->regionsTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

void RegionDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::regionDialog_geometry);
}





static int openRegionDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Region* originalRegion);

int openNewRegionDialogAndStore(QWidget* parent, Database* db)
{
	return openRegionDialogAndStore(parent, db, newItem, nullptr);
}

void openEditRegionDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Region* originalRegion = db->getRegionAt(bufferRowIndex);
	openRegionDialogAndStore(parent, db, editItem, originalRegion);
}

void openDeleteRegionDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex)
{
	Region* region = db->getRegionAt(bufferRowIndex);
	ValidItemID regionID = region->regionID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->regionsTable, regionID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = RegionDialog::tr("Delete region");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->regionsTable, regionID);
}



static int openRegionDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Region* originalRegion)
{
	int newRegionIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalRegion);
		originalRegion->regionID = ItemID();
	}
	
	RegionDialog dialog(parent, db, purpose, originalRegion);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Region* extractedRegion = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newRegionIndex = db->regionsTable->addRow(parent, extractedRegion);
			break;
		case editItem:
			db->regionsTable->updateRow(parent, originalRegion->regionID.forceValid(), extractedRegion);
			break;
		default:
			assert(false);
		}
		
		delete extractedRegion;
	}
	
	return newRegionIndex;
}
