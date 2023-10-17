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

#include "peak_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/region_dialog.h"
#include "src/main/settings.h"

#include <QMessageBox>



PeakDialog::PeakDialog(QWidget* parent, Database* db, DialogPurpose purpose, Peak* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init),
		selectableRegionIDs(QList<ValidItemID>())
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/peak_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::peakDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(heightCheckbox,		&QCheckBox::stateChanged,	this,	&PeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,	&QPushButton::clicked,		this,	&PeakDialog::handle_newRegion);
	
	connect(okButton,			&QPushButton::clicked,		this,	&PeakDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,		this,	&PeakDialog::handle_cancel);
	
	
	// Set initial height
	heightCheckbox->setChecked(Settings::peakDialog_heightEnabledInitially.get());
	handle_heightSpecifiedChanged();
	heightSpinner->setValue(Settings::peakDialog_initialHeight.get());
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	case duplicateItem:
		Peak* blankPeak = extractData();
		insertInitData();
		this->init = blankPeak;
		break;
	}
}

PeakDialog::~PeakDialog()
{
	delete init;
}



QString PeakDialog::getEditWindowTitle()
{
	return tr("Edit peak");
}



void PeakDialog::populateComboBoxes()
{
	populateItemCombo(db->regionsTable, db->regionsTable->nameColumn, true, regionCombo, selectableRegionIDs);
}



void PeakDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Height
	bool heightSpecified = init->heightSpecified();
	heightCheckbox->setChecked(heightSpecified);
	if (heightSpecified) {
		heightSpinner->setValue(init->height);
	}	
	handle_heightSpecifiedChanged();
	// Volcano
	volcanoCheckbox->setChecked(init->volcano);
	// Region
	if (init->regionID.isValid()) {
		regionCombo->setCurrentIndex(selectableRegionIDs.indexOf(init->regionID.get()) + 1);	// 0 is None
	} else {
		regionCombo->setCurrentIndex(0);
	}
	// Links
	googleMapsLineEdit->setText(init->mapsLink);
	googleEarthLineEdit->setText(init->earthLink);
	wikipediaLineEdit->setText(init->wikiLink);
}


Peak* PeakDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	int		height		= parseSpinner		(heightSpinner);
	bool	volcano		= parseCheckbox		(volcanoCheckbox);
	ItemID	regionID	= parseItemCombo	(regionCombo, selectableRegionIDs);
	QString	mapsLink	= parseLineEdit		(googleMapsLineEdit);
	QString	earthLink	= parseLineEdit		(googleEarthLineEdit);
	QString	wikiLink	= parseLineEdit		(wikipediaLineEdit);
	
	if (!heightCheckbox->isChecked())	height = -1;
	
	Peak* peak = new Peak(ItemID(), name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
	return peak;
}


bool PeakDialog::changesMade()
{
	Peak* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void PeakDialog::handle_heightSpecifiedChanged()
{
	bool enabled = heightCheckbox->isChecked();
	heightSpinner->setEnabled(enabled);
}

void PeakDialog::handle_newRegion()
{
	int newRegionIndex = openNewRegionDialogAndStore(this, db);
	if (newRegionIndex < 0) return;
	
	populateItemCombo(db->regionsTable, db->regionsTable->nameColumn, true, regionCombo, selectableRegionIDs);
	ValidItemID regionID = db->regionsTable->getPrimaryKeyAt(newRegionIndex);
	regionCombo->setCurrentIndex(selectableRegionIDs.indexOf(regionID) + 1);	// 0 is None
}



void PeakDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save peak");
	QString emptyNameWindowMessage	= tr("The peak needs a name.");
	const Column* nameColumn = db->peaksTable->nameColumn;
	NewOrEditDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

void PeakDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::peakDialog_geometry);
}





static int openPeakDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Peak* originalPeak);

int openNewPeakDialogAndStore(QWidget* parent, Database* db)
{
	return openPeakDialogAndStore(parent, db, newItem, nullptr);
}

int openDuplicatePeakDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Peak* originalPeak = db->getPeakAt(bufferRowIndex);
	return openPeakDialogAndStore(parent, db, duplicateItem, originalPeak);
}

void openEditPeakDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Peak* originalPeak = db->getPeakAt(bufferRowIndex);
	openPeakDialogAndStore(parent, db, editItem, originalPeak);
}

void openDeletePeakDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex)
{
	Peak* peak = db->getPeakAt(bufferRowIndex);
	ValidItemID peakID = peak->peakID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->peaksTable, peakID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = PeakDialog::tr("Delete peak");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->peaksTable, peakID);
}



static int openPeakDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Peak* originalPeak)
{
	int newPeakIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalPeak);
		originalPeak->peakID = ItemID();
	}
	
	PeakDialog dialog(parent, db, purpose, originalPeak);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Peak* extractedPeak = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newPeakIndex = db->peaksTable->addRow(parent, extractedPeak);
			break;
		case editItem:
			db->peaksTable->updateRow(parent, originalPeak->peakID.forceValid(), extractedPeak);
			break;
		default:
			assert(false);
		}
		
		delete extractedPeak;
	}
	
	return newPeakIndex;
}
