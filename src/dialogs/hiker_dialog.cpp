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

using std::unique_ptr, std::make_unique;



/**
 * Creates a new hiker dialog.
 * 
 * Sets up the UI, restores geometry, connects interactive UI elements, and performs purpose-
 * specific preparations.
 * 
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param windowTitle	The title of the dialog window.
 * @param init			The hiker data to initialize the dialog with and store as initial data. HikerDialog takes ownership of this pointer.
 */
HikerDialog::HikerDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, const QString& windowTitle, unique_ptr<const Hiker> init) :
	ItemDialog(parent, mainWindow, db, purpose, windowTitle),
	init(std::move(init))
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,	{ nameLineEdit }}
	});
	
	setWindowIcon(QIcon(":/icons/ico/hiker_multisize_square.ico"));
	
	restoreDialogGeometry(*this, mainWindow, Settings::hikerDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,		this,	&HikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&HikerDialog::handle_cancel);
	
	
	changeUIForPurpose();
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		insertInitData();
		break;
	case multiEdit:
		// TODO
		break;
	default:
		assert(false);
	}
}

/**
 * Destroys the hiker dialog.
 */
HikerDialog::~HikerDialog()
{}



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
unique_ptr<Hiker> HikerDialog::extractData()
{
	QString	name	= parseLineEdit	(*nameLineEdit);
	
	return make_unique<Hiker>(ItemID(), name);
}


/**
 * Checks whether changes have been made to the hiker, compared to the initial hiker object, if
 * set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool HikerDialog::changesMade()
{
	return !extractData()->equalTo(*init);
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
	const ValueColumn& nameColumn = db.hikersTable.nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void HikerDialog::aboutToClose()
{
	saveDialogGeometry(*this, mainWindow, Settings::hikerDialog_geometry);
}





/**
 * Opens a new hiker dialog and saves the new hiker to the database.
 *
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @return				The index of the new hiker in the database's hiker table buffer.
 */
BufferRowIndex openNewHikerDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db)
{
	const QString windowTitle = HikerDialog::tr("New hiker");
	
	HikerDialog dialog = HikerDialog(parent, mainWindow, db, newItem, windowTitle, nullptr);
	if (dialog.exec() != QDialog::Accepted) {
		return BufferRowIndex();
	}
	
	unique_ptr<Hiker> extractedHiker = dialog.extractData();
	
	const BufferRowIndex newHikerIndex = db.hikersTable.addRow(parent, *extractedHiker);
	return newHikerIndex;
}

/**
 * Opens an edit hiker dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the hiker to edit in the database's hiker table buffer.
 * @return					True if any changes were made, false otherwise.
 */
bool openEditHikerDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex)
{
	unique_ptr<Hiker> originalHiker = db.getHikerAt(bufferRowIndex);
	const ItemID originalHikerID = originalHiker->hikerID;
	
	const QString windowTitle = HikerDialog::tr("Edit hiker");
	
	HikerDialog dialog = HikerDialog(parent, mainWindow, db, editItem, windowTitle, std::move(originalHiker));
	if (dialog.exec() != QDialog::Accepted || !dialog.changesMade()) {
		return false;
	}
	
	unique_ptr<Hiker> extractedHiker = dialog.extractData();
	
	db.hikersTable.updateRow(parent, FORCE_VALID(originalHikerID), *extractedHiker);
	return true;
}

/**
 * Opens a delete hiker dialog and deletes the hiker from the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the hikers to delete in the database's hiker table buffer.
 * @return					True if any items were deleted, false otherwise.
 */
bool openDeleteHikersDialogAndExecute(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices)
{
	Q_UNUSED(mainWindow);
	if (bufferRowIndices.isEmpty()) return false;
	
	QSet<ValidItemID> hikerIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		hikerIDs += VALID_ITEM_ID(db.hikersTable.primaryKeyColumn.getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db.whatIf_removeRows(db.hikersTable, hikerIDs);
	
	if (Settings::confirmDelete.get()) {
		bool plural = hikerIDs.size() > 1;
		QString windowTitle = plural ? HikerDialog::tr("Delete hikers") : HikerDialog::tr("Delete hiker");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}
	
	for (const ItemID& hikerID : qAsConst(hikerIDs)) {
		if (db.projectSettings.defaultHiker.get() == ID_GET(hikerID)) {
			db.projectSettings.defaultHiker.clear(parent);
			break;
		}
	}
	
	db.removeRows(parent, db.hikersTable, hikerIDs);
	return true;
}
