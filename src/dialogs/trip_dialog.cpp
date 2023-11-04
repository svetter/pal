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

/**
 * @file trip_dialog.cpp
 * 
 * This file defines the TripDialog class.
 */

#include "trip_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



/**
 * Creates a new trip dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, sets
 * initial values, and performs purpose-specific preparations.
 * 
 * @param parent	The parent window.
 * @param db		The project database.
 * @param purpose	The purpose of the dialog.
 * @param init		The trip data to initialize the dialog with and store as initial data. TripDialog takes ownership of this pointer.
 */
TripDialog::TripDialog(QWidget* parent, Database* db, DialogPurpose purpose, Trip* init) :
		ItemDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	setWindowIcon(QIcon(":/icons/ico/trip_multisize_square.ico"));
	
	restoreDialogGeometry(this, parent, &Settings::tripDialog_geometry);
	
	
	connect(datesUnspecifiedCheckbox,	&QCheckBox::stateChanged,	this,	&TripDialog::handle_datesSpecifiedChanged);	
	connect(startDateWidget,			&QDateEdit::dateChanged,	this,	&TripDialog::handle_startDateChanged);	
	connect(endDateWidget,				&QDateEdit::dateChanged,	this,	&TripDialog::handle_endDateChanged);
	
	connect(okButton,					&QPushButton::clicked,		this,	&TripDialog::handle_ok);
	connect(cancelButton,				&QPushButton::clicked,		this,	&TripDialog::handle_cancel);
	
	
	// Set initial dates
	QDate initialDate = QDateTime::currentDateTime().date();
	startDateWidget->setDate(initialDate);
	endDateWidget->setDate(initialDate);
	datesUnspecifiedCheckbox->setChecked(!Settings::tripDialog_datesEnabledInitially.get());
	handle_datesSpecifiedChanged();
	
	
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
 * Destroys the trip dialog.
 */
TripDialog::~TripDialog()
{
	delete init;
}



/**
 * Returns the window title to use when the dialog is used to edit an item.
 *
 * @return	The window title for editing an item
 */
QString TripDialog::getEditWindowTitle()
{
	return tr("Edit trip");
}



/**
 * Inserts the data from the initial trip object into the dialog's UI elements.
 */
void TripDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Start/end date
	bool datesSpecified = init->datesSpecified();
	datesUnspecifiedCheckbox->setChecked(!datesSpecified);
	if (datesSpecified) {
		startDateWidget->setDate(init->startDate);
		endDateWidget->setDate(init->endDate);
	}	
	handle_datesSpecifiedChanged();
	// Description
	descriptionEditor->setPlainText(init->description);
}


/**
 * Extracts the data from the UI elements and returns it as a trip object.
 *
 * @return	The trip data as a trip object. The caller takes ownership of the object.
 */
Trip* TripDialog::extractData()
{
	QString	name		= parseLineEdit			(nameLineEdit);
	QDate	startDate	= parseDateWidget		(startDateWidget);
	QDate	endDate		= parseDateWidget		(endDateWidget);
	QString	description	= parsePlainTextEdit	(descriptionEditor);
	
	if (datesUnspecifiedCheckbox->isChecked())	startDate = QDate();	
	if (datesUnspecifiedCheckbox->isChecked())	endDate = QDate();
	
	Trip* trip = new Trip(ItemID(), name, startDate, endDate, description);
	return trip;
}


/**
 * Checks whether changes have been made to the trip, compared to the initial trip object, if set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool TripDialog::changesMade()
{
	Trip* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



/**
 * Event handler for changes in the dates specified checkbox.
 *
 * Enables or disables the date widgets and labels according to the checkbox's state.
 */
void TripDialog::handle_datesSpecifiedChanged()
{
	bool enabled = !datesUnspecifiedCheckbox->isChecked();
	startDateLabel->setEnabled(enabled);
	endDateLabel->setEnabled(enabled);
	startDateWidget->setEnabled(enabled);
	endDateWidget->setEnabled(enabled);
}


/**
 * Event handler for changes in the start date.
 *
 * Ensures that the start date is not later than the end date.
 */
void TripDialog::handle_startDateChanged()
{
	if (endDateWidget->date() < startDateWidget->date()) {
		endDateWidget->setDate(startDateWidget->date());
	}
}

/**
 * Event handler for changes in the end date.
 *
 * Ensures that the end date is not earlier than the start date.
 */
void TripDialog::handle_endDateChanged()
{
	if (startDateWidget->date() > endDateWidget->date()) {
		startDateWidget->setDate(endDateWidget->date());
	}
}



/**
 * Event handler for the OK button.
 *
 * Checks whether the name is empty or a duplicate depending on settings, prepares the dialog for
 * closing and then accepts it.
 */
void TripDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save trip");
	QString emptyNameWindowMessage	= tr("The trip needs a name.");
	const Column* nameColumn = db->tripsTable->nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void TripDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::tripDialog_geometry);
}





static BufferRowIndex openTripDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Trip* originalTrip);

/**
 * Opens a new trip dialog and saves the new trip to the database.
 *
 * @param parent	The parent window.
 * @param db		The project database.
 * @return			The index of the new trip in the database's trip table buffer.
 */
BufferRowIndex openNewTripDialogAndStore(QWidget* parent, Database* db)
{
	return openTripDialogAndStore(parent, db, newItem, nullptr);
}

/**
 * Opens an edit trip dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the trip to edit in the database's trip table buffer.
 */
void openEditTripDialogAndStore(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Trip* originalTrip = db->getTripAt(bufferRowIndex);
	openTripDialogAndStore(parent, db, editItem, originalTrip);
}

/**
 * Opens a delete trip dialog and deletes the trip from the database.
 *
 * @param parent			The parent window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the trip to delete in the database's trip table buffer.
 */
void openDeleteTripDialogAndExecute(QWidget* parent, Database* db, BufferRowIndex bufferRowIndex)
{
	Trip* trip = db->getTripAt(bufferRowIndex);
	ValidItemID tripID = trip->tripID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->tripsTable, tripID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = TripDialog::tr("Delete trip");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->tripsTable, tripID);
}



/**
 * Opens a purpose-generic trip dialog and applies the resulting changes to the database.
 *
 * @param parent		The parent window.
 * @param db			The project database.
 * @param purpose		The purpose of the dialog.
 * @param originalTrip	The trip data to initialize the dialog with and store as initial data. TripDialog takes ownership of this pointer.
 * @return				The index of the new trip in the database's trip table buffer. Invalid if the dialog was canceled or the purpose was editItem.
 */
static BufferRowIndex openTripDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Trip* originalTrip)
{
	BufferRowIndex newTripIndex = BufferRowIndex();
	if (purpose == duplicateItem) {
		assert(originalTrip);
		originalTrip->tripID = ItemID();
	}
	
	TripDialog dialog(parent, db, purpose, originalTrip);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Trip* extractedTrip = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newTripIndex = db->tripsTable->addRow(parent, extractedTrip);
			break;
		case editItem:
			db->tripsTable->updateRow(parent, originalTrip->tripID.forceValid(), extractedTrip);
			break;
		default:
			assert(false);
		}
		
		delete extractedTrip;
	}
	
	return newTripIndex;
}
