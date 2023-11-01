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

#include "hiker_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



HikerDialog::HikerDialog(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* init) :
		ItemDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/hiker_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::hikerDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,		this,	&HikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&HikerDialog::handle_cancel);
	
	
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

HikerDialog::~HikerDialog()
{
	delete init;
}



QString HikerDialog::getEditWindowTitle()
{
	return tr("Edit hiker");
}



void HikerDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
}


Hiker* HikerDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	
	Hiker* hiker = new Hiker(ItemID(), name);
	return hiker;
}


bool HikerDialog::changesMade()
{
	Hiker* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void HikerDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save hiker");
	QString emptyNameWindowMessage	= tr("The hiker needs a name.");
	const Column* nameColumn = db->hikersTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

void HikerDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::hikerDialog_geometry);
}





static BufferRowIndex openHikerDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* originalHiker);

BufferRowIndex openNewHikerDialogAndStore(QWidget* parent, Database* db)
{
	return openHikerDialogAndStore(parent, db, newItem, nullptr);
}

void openEditHikerDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Hiker* originalHiker = db->getHikerAt(bufferRowIndex);
	openHikerDialogAndStore(parent, db, editItem, originalHiker);
}

void openDeleteHikerDialogAndExecute(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Hiker* hiker = db->getHikerAt(bufferRowIndex);
	ValidItemID hikerID = hiker->hikerID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->hikersTable, hikerID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = HikerDialog::tr("Delete hiker");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	if (db->projectSettings->defaultHiker->get() == hikerID) {
		db->projectSettings->defaultHiker->setToNull(parent);
	}
	
	db->removeRow(parent, db->hikersTable, hikerID);
}



static BufferRowIndex openHikerDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* originalHiker)
{
	BufferRowIndex newHikerIndex = BufferRowIndex();
	if (purpose == duplicateItem) {
		assert(originalHiker);
		originalHiker->hikerID = ItemID();
	}
	
	HikerDialog dialog(parent, db, purpose, originalHiker);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Hiker* extractedHiker = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newHikerIndex = db->hikersTable->addRow(parent, extractedHiker);
			break;
		case editItem:
			db->hikersTable->updateRow(parent, originalHiker->hikerID.forceValid(), extractedHiker);
			break;
		default:
			assert(false);
		}
		
		delete extractedHiker;
	}
	
	return newHikerIndex;
}
