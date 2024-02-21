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
 * @file add_hiker_dialog.cpp
 * 
 * This file defines the AddHikerDialog class.
 */

#include "add_hiker_dialog.h"

#include "src/dialogs/hiker_dialog.h"

#include <QMessageBox>



/**
 * Creates a new add hiker dialog.
 * 
 * Sets up UI, populates combo boxes and connects interactive elements.
 * 
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 */
AddHikerDialog::AddHikerDialog(QWidget* parent, QMainWindow* mainWindow, Database* db) :
	QDialog(parent),
	mainWindow(mainWindow),
	db(db),
	selectableHikerIDs(QList<ValidItemID>())
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(newHikerButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_newHiker);
	
	connect(okButton,		&QPushButton::clicked,	this,	&AddHikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_cancel);
}



/**
 * Populates the combo boxes with items from the database.
 */
void AddHikerDialog::populateComboBoxes()
{
	populateItemCombo(db->hikersTable, db->hikersTable.nameColumn, true, hikerCombo, selectableHikerIDs);
}



/**
 * Extracts the ID of the hiker selected in the dialog.
 * 
 * @return	The selected hiker's ID.
 */
ValidItemID AddHikerDialog::extractHikerID()
{
	assert(hikerSelected());
	return selectableHikerIDs.at(hikerCombo->currentIndex() - 1);
}


/**
 * Checks whether a hiker is selected in the dialog.
 * 
 * @return	True if a hiker is selected, false otherwise.
 */
bool AddHikerDialog::hikerSelected()
{
	return hikerCombo->currentIndex() > 0;
}



/**
 * Event handler for the new hiker button.
 * 
 * Opens a new hiker dialog and selects the new hiker in the combo box if one was created.
 */
void AddHikerDialog::handle_newHiker()
{
	BufferRowIndex newHikerIndex = openNewHikerDialogAndStore(this, mainWindow, db);
	if (newHikerIndex.isInvalid()) return;
	
	populateItemCombo(db->hikersTable, db->hikersTable.nameColumn, true, hikerCombo, selectableHikerIDs);
	const ValidItemID newHikerID = db->hikersTable.getPrimaryKeyAt(newHikerIndex);
	hikerCombo->setCurrentIndex(selectableHikerIDs.indexOf(newHikerID) + 1);	// 0 is None
}


/**
 * Event handler for the ok button.
 * 
 * Accepts the dialog if a hiker is selected, otherwise shows an error message.
 */
void AddHikerDialog::handle_ok()
{
	if (hikerSelected()) {
		accept();
	} else {
		QString title = tr("Can't add hiker");
		QString message = tr("You have to select a hiker.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

/**
 * Event handler for the cancel button.
 * 
 * Rejects the dialog without doing anything else.
 */
void AddHikerDialog::handle_cancel()
{
	reject();
}



/**
 * Opens a new add hiker dialog and returns the ID of the selected hiker or an invalid ItemID if
 * the dialog was cancelled.
 * 
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 */
ItemID openAddHikerDialog(QWidget* parent, QMainWindow* mainWindow, Database* db)
{
	AddHikerDialog dialog(parent, mainWindow, db);
	if (dialog.exec() == QDialog::Accepted) {
		return dialog.extractHikerID();
	} else {
		return -1;
	}
}
