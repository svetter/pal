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

/**
 * @file hiker_dialog.cpp
 * 
 * This file defines the HikerDialog class.
 */

#include "hiker_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/settings/settings.h"

#include <QMessageBox>



/**
 * Creates a new hiker dialog.
 * 
 * Sets up the UI, restores geometry, connects interactive UI elements, and performs purpose-
 * specific preparations.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 * @param init		The hiker data to initialize the dialog with and store as initial data. HikerDialog takes ownership of this pointer.
 */
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

/**
 * Destroys the hiker dialog.
 */
HikerDialog::~HikerDialog()
{
	delete init;
}



/**
 * Returns the window title to use when the dialog is used to edit an item.
 *
 * @return	The window title for editing an item
 */
QString HikerDialog::getEditWindowTitle()
{
	return tr("Edit hiker");
}



/**
 * Inserts the data from the initial hiker object into the dialog's UI elements.
 */
void HikerDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
}


/**
 * Extracts the data from the UI elements and returns it as a hiker object.
 *
 * @return	The hiker data as a hiker object. The caller takes ownership of the object.
 */
Hiker* HikerDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	
	Hiker* hiker = new Hiker(ItemID(), name);
	return hiker;
}


/**
 * Checks whether changes have been made to the hiker, compared to the initial hiker object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool HikerDialog::changesMade()
{
	Hiker* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



/**
 * Event handler for the OK button.
 *
 * Checks whether the name is empty or a duplicate depending on settings, prepares the dialog for
 * closing and then accepts it.
 */
void HikerDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save hiker");
	QString emptyNameWindowMessage	= tr("The hiker needs a name.");
	const ValueColumn* nameColumn = db->hikersTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void HikerDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::hikerDialog_geometry);
}





static BufferRowIndex openHikerDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Hiker* originalHiker);

/**
 * Opens a new hiker dialog and saves the new hiker to the database.
 *
 * @param parent	The parent window.
 * @param db		The project database.
 * @return			The index of the new hiker in the database's hiker table buffer.
 */
BufferRowIndex openNewHikerDialogAndStore(QWidget* parent, Database* db)
{
	return openHikerDialogAndStore(parent, db, newItem, nullptr);
}

/**
 * Opens an edit hiker dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the hiker to edit in the database's hiker table buffer.
 */
void openEditHikerDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Hiker* originalHiker = db->getHikerAt(bufferRowIndex);
	openHikerDialogAndStore(parent, db, editItem, originalHiker);
}

/**
 * Opens a delete hiker dialog and deletes the hiker from the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the hikers to delete in the database's hiker table buffer.
 */
void openDeleteHikersDialogAndExecute(QWidget* parent, Database* db, QSet<BufferRowIndex> bufferRowIndices)
{
	if (bufferRowIndices.isEmpty()) return;
	
	QSet<ValidItemID> hikerIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		hikerIDs += VALID_ITEM_ID(db->hikersTable->primaryKeyColumn->getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRows(db->hikersTable, hikerIDs);
	
	if (Settings::confirmDelete.get()) {
		bool plural = hikerIDs.size() > 1;
		QString windowTitle = plural ? HikerDialog::tr("Delete hikers") : HikerDialog::tr("Delete hiker");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}
	
	for (const ItemID& hikerID : qAsConst(hikerIDs)) {
		if (db->projectSettings->defaultHiker.get() == ID_GET(hikerID)) {
			db->projectSettings->defaultHiker.clear(parent);
			break;
		}
	}
	
	db->removeRows(parent, db->hikersTable, hikerIDs);
}



/**
 * Opens a purpose-generic hiker dialog and applies the resulting changes to the database.
 *
 * @param parent		The parent window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param originalHiker	The hiker data to initialize the dialog with and store as initial data. HikerDialog takes ownership of this pointer.
 * @return				The index of the new hiker in the database's hiker table buffer. Invalid if the dialog was canceled or the purpose was editItem.
 */
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
			db->hikersTable->updateRow(parent, FORCE_VALID(originalHiker->hikerID), extractedHiker);
			break;
		default:
			assert(false);
		}
		
		delete extractedHiker;
	}
	
	return newHikerIndex;
}
