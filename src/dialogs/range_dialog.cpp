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

#include "range_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



RangeDialog::RangeDialog(QWidget* parent, Database* db, DialogPurpose purpose, Range* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	
	restoreDialogGeometry(this, parent, &Settings::rangeDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
	
	
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

RangeDialog::~RangeDialog()
{
	delete init;
}



QString RangeDialog::getEditWindowTitle()
{
	return tr("Edit mountain range");
}



void RangeDialog::populateComboBoxes()
{
	continentCombo->insertItems(0, Range::continentNames);
}



void RangeDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Continent
	continentCombo->setCurrentIndex(init->continent);
}


Range* RangeDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	int		continent	= parseEnumCombo	(continentCombo, true);
	
	Range* range = new Range(ItemID(), name, continent);
	return range;
}


bool RangeDialog::changesMade()
{
	Range* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void RangeDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save mountain range");
	QString emptyNameWindowMessage	= tr("The mountain range needs a name.");
	const Column* nameColumn = db->rangesTable->nameColumn;
	NewOrEditDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

void RangeDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::rangeDialog_geometry);
}





static int openRangeDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Range* originalRange);

int openNewRangeDialogAndStore(QWidget* parent, Database* db)
{
	return openRangeDialogAndStore(parent, db, newItem, nullptr);
}

void openEditRangeDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Range* originalRange = db->getRangeAt(bufferRowIndex);
	openRangeDialogAndStore(parent, db, editItem, originalRange);
}

void openDeleteRangeDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex)
{
	Range* range = db->getRangeAt(bufferRowIndex);
	ValidItemID rangeID = range->rangeID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->rangesTable, rangeID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = RangeDialog::tr("Delete mountain range");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->rangesTable, rangeID);
}



static int openRangeDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Range* originalRange)
{
	int newRangeIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalRange);
		originalRange->rangeID = ItemID();
	}
	
	RangeDialog dialog(parent, db, purpose, originalRange);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Range* extractedRange = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newRangeIndex = db->rangesTable->addRow(parent, extractedRange);
			break;
		case editItem:
			db->rangesTable->updateRow(parent, originalRange->rangeID.forceValid(), extractedRange);
			break;
		default:
			assert(false);
		}
		
		delete extractedRange;
	}
	
	return newRangeIndex;
}
