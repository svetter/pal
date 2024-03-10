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
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param init				The hiker data to initialize the dialog with and store as initial data. HikerDialog takes ownership of this pointer.
 * @param numItemsToEdit	The number of items to edit, if the purpose is multi-edit.
 */
HikerDialog::HikerDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Hiker> init, int numItemsToEdit) :
	ItemDialog(parent, mainWindow, db, purpose),
	init(std::move(init))
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,	{{ nameLineEdit }, { &db.hikersTable.nameColumn }}}
	});
	
	setWindowIcon(QIcon(":/icons/ico/hiker_multisize_square.ico"));
	switch (purpose) {
	case newItem:
	case duplicateItem:	setWindowTitle(tr("New hiker"));								break;
	case editItem:
	case multiEdit:		setWindowTitle(tr("Edit %Ln hiker(s)", "", numItemsToEdit));	break;
	}
	
	restoreDialogGeometry(*this, mainWindow, Settings::hikerDialog_geometry);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,		this,	&HikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&HikerDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
	case multiEdit:
		insertInitData();
		break;
	default:
		assert(false);
	}
	changeUIForPurpose();
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
	if (purpose == multiEdit) {
		return anyMultiEditChanges();
	}
	
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
 * @param callWhenDone	The function to call after the dialog has closed.
 * @return				The index of the new hiker in the database's hiker table buffer.
 */
void openNewHikerDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, std::function<void (BufferRowIndex)> callWhenDone)
{
	HikerDialog* dialog = new HikerDialog(parent, mainWindow, db, newItem, nullptr);
	
	auto callWhenClosed = [=, &parent, &db]() {
		BufferRowIndex newHikerIndex = BufferRowIndex();
		
		if (dialog->result() == QDialog::Accepted) {
			unique_ptr<Hiker> extractedHiker = dialog->extractData();
			
			db.beginChangingData();
			newHikerIndex = db.hikersTable.addRow(parent, *extractedHiker);
			db.finishChangingData();
		}
		
		delete dialog;
		return callWhenDone(newHikerIndex);
	};
	HikerDialog::connect(dialog, &HikerDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens an edit hiker dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the hiker to edit in the database's hiker table buffer.
 * @param callWhenDone		The function to call after the dialog has closed.
 * @return					True if any changes were made, false otherwise.
 */
void openEditHikerDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (bool)> callWhenDone)
{
	unique_ptr<Hiker> originalHiker = db.getHikerAt(bufferRowIndex);
	const ItemID originalHikerID = originalHiker->hikerID;
	
	HikerDialog* dialog = new HikerDialog(parent, mainWindow, db, editItem, std::move(originalHiker));
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Hiker> extractedHiker = dialog->extractData();
			
			db.beginChangingData();
			db.hikersTable.updateRow(parent, FORCE_VALID(originalHikerID), *extractedHiker);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	HikerDialog::connect(dialog, &HikerDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a multi-edit hiker dialog and saves the changes to the database.
 * 
 * @param parent				The parent window.
 * @param mainWindow			The application's main window.
 * @param db					The project database.
 * @param bufferRowIndices		The buffer row indices of the hikers to edit.
 * @param initBufferRowIndex	The index of the hiker whose data to initialize the dialog with.
 * @param callWhenDone			The function to call after the dialog has closed.
 * @return						True if any changes were made, false otherwise.
 */
void openMultiEditHikersDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices, BufferRowIndex initBufferRowIndex, std::function<void (bool)> callWhenDone)
{
	assert(!bufferRowIndices.isEmpty());
	
	unique_ptr<Hiker> originalHiker = db.getHikerAt(initBufferRowIndex);
	
	HikerDialog* dialog = new HikerDialog(parent, mainWindow, db, multiEdit, std::move(originalHiker), bufferRowIndices.size());
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Hiker> extractedHiker = dialog->extractData();
			extractedHiker->hikerID = ItemID();
			QSet<const Column*> columnsToSave = dialog->getMultiEditColumns();
			QList<const Column*> columnList = QList<const Column*>(columnsToSave.constBegin(), columnsToSave.constEnd());
			
			db.beginChangingData();
			db.hikersTable.updateRows(parent, bufferRowIndices, columnList, *extractedHiker);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	HikerDialog::connect(dialog, &HikerDialog::finished, callWhenClosed);
	
	dialog->open();
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
		const QString windowTitle = HikerDialog::tr("Delete %Ln hiker(s)", "", hikerIDs.size());
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}
	
	db.beginChangingData();
	for (const ItemID& hikerID : qAsConst(hikerIDs)) {
		if (db.projectSettings.defaultHiker.get() == ID_GET(hikerID)) {
			db.projectSettings.defaultHiker.clear(parent);
			break;
		}
	}
	db.removeRows(parent, db.hikersTable, hikerIDs);
	db.finishChangingData();
	return true;
}
