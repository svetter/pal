/*
 * Copyright 2023-2025 Simon Vetter
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
#include "src/settings/settings.h"

#include <QMessageBox>

using std::unique_ptr, std::make_unique;



/**
 * Creates a new trip dialog.
 * 
 * Sets up the UI, restores geometry, populates combo boxes, connects interactive UI elements, sets
 * initial values, and performs purpose-specific preparations.
 * 
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param purpose			The purpose of the dialog.
 * @param init				The trip data to initialize the dialog with and store as initial data. TripDialog takes ownership of this pointer.
 * @param numItemsToEdit	The number of items to edit, if the purpose is multi-edit.
 */
TripDialog::TripDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Trip> init, int numItemsToEdit) :
	ItemDialog(parent, mainWindow, db, purpose),
	init(std::move(init))
{
	setupUi(this);
	setUIPointers(okButton, {
		{nameCheckbox,			{{ nameLineEdit },																				{ &db.tripsTable.nameColumn }}},
		{datesCheckbox,			{{ startDateLabel, startDateWidget, endDateLabel, endDateWidget, datesUnspecifiedCheckbox },	{ &db.tripsTable.startDateColumn, &db.tripsTable.endDateColumn }}},
		{descriptionCheckbox,	{{ descriptionEditor },																			{ &db.tripsTable.descriptionColumn }}}
	});
	
	setWindowIcon(QIcon(":/icons/ico/trip_multisize_square.ico"));
	switch (purpose) {
	case newItem:
	case duplicateItem:	setWindowTitle(tr("New trip"));								break;
	case editItem:
	case multiEdit:		setWindowTitle(tr("Edit %Ln trip(s)", "", numItemsToEdit));	break;
	}
	setSizeGripEnabled(true);
	
	restoreDialogGeometry(*this, mainWindow, Settings::tripDialog_geometry);
	
	
	connect(datesUnspecifiedCheckbox,	&QCheckBox::checkStateChanged,	this,	&TripDialog::handle_datesSpecifiedChanged);
	connect(startDateWidget,			&QDateEdit::dateChanged,		this,	&TripDialog::handle_startDateChanged);
	connect(endDateWidget,				&QDateEdit::dateChanged,		this,	&TripDialog::handle_endDateChanged);
	
	connect(okButton,					&QPushButton::clicked,			this,	&TripDialog::handle_ok);
	connect(cancelButton,				&QPushButton::clicked,			this,	&TripDialog::handle_cancel);
	
	
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
	case multiEdit:
		insertInitData();
		break;
	default:
		assert(false);
	}
	changeUIForPurpose();
}

/**
 * Destroys the trip dialog.
 */
TripDialog::~TripDialog()
{}



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
	// Description
	descriptionEditor->setPlainText(init->description);
}


/**
 * Extracts the data from the UI elements and returns it as a trip object.
 *
 * @return	The trip data as a trip object. The caller takes ownership of the object.
 */
unique_ptr<Trip> TripDialog::extractData()
{
	QString	name		= parseLineEdit			(*nameLineEdit);
	QDate	startDate	= parseDateWidget		(*startDateWidget);
	QDate	endDate		= parseDateWidget		(*endDateWidget);
	QString	description	= parsePlainTextEdit	(*descriptionEditor);
	
	if (datesUnspecifiedCheckbox->isChecked())	startDate = QDate();
	if (datesUnspecifiedCheckbox->isChecked())	endDate = QDate();
	
	return make_unique<Trip>(ItemID(), name, startDate, endDate, description);
}


/**
 * Checks whether changes have been made to the trip, compared to the initial trip object, if set.
 *
 * @return	True if the current UI contents are different from their initial state, false otherwise.
 */
bool TripDialog::changesMade()
{
	if (purpose == multiEdit) {
		return anyMultiEditChanges();
	}
	
	return !extractData()->equalTo(*init);
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
	const ValueColumn& nameColumn = db.tripsTable.nameColumn;
	ItemDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

/**
 * Prepares the dialog for closing by saving its geometry.
 */
void TripDialog::aboutToClose()
{
	saveDialogGeometry(*this, mainWindow, Settings::tripDialog_geometry);
}





/**
 * Opens a new trip dialog and saves the new trip to the database.
 *
 * @param parent		The parent window.
 * @param mainWindow	The application's main window.
 * @param db			The project database.
 * @param callWhenDone	The function to call after the dialog has closed.
 */
void openNewTripDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, std::function<void (BufferRowIndex)> callWhenDone)
{
	TripDialog* dialog = new TripDialog(parent, mainWindow, db, newItem, nullptr);
	
	auto callWhenClosed = [=, &parent, &db]() {
		BufferRowIndex newTripIndex = BufferRowIndex();
		
		if (dialog->result() == QDialog::Accepted) {
			unique_ptr<Trip> extractedTrip = dialog->extractData();
			
			db.beginChangingData();
			newTripIndex = db.tripsTable.addRow(parent, *extractedTrip);
			db.finishChangingData();
		}
		
		delete dialog;
		return callWhenDone(newTripIndex);
	};
	TripDialog::connect(dialog, &TripDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens an edit trip dialog and saves the changes to the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndex	The index of the trip to edit in the database's trip table buffer.
 * @param callWhenDone		The function to call after the dialog has closed.
 */
void openEditTripDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (bool)> callWhenDone)
{
	unique_ptr<Trip> originalTrip = db.getTripAt(bufferRowIndex);
	const ItemID originalTripID = originalTrip->tripID;
	
	TripDialog* dialog = new TripDialog(parent, mainWindow, db, editItem, std::move(originalTrip));
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Trip> extractedTrip = dialog->extractData();
			
			db.beginChangingData();
			db.tripsTable.updateRow(parent, FORCE_VALID(originalTripID), *extractedTrip);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	TripDialog::connect(dialog, &TripDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a multi-edit trip dialog and saves the changes to the database.
 * 
 * @param parent				The parent window.
 * @param mainWindow			The application's main window.
 * @param db					The project database.
 * @param bufferRowIndices		The buffer row indices of the trips to edit.
 * @param initBufferRowIndex	The index of the trip whose data to initialize the dialog with.
 * @param callWhenDone			The function to call after the dialog has closed.
 */
void openMultiEditTripsDialogAndStore(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices, BufferRowIndex initBufferRowIndex, std::function<void (bool)> callWhenDone)
{
	assert(!bufferRowIndices.isEmpty());
	
	unique_ptr<Trip> originalTrip = db.getTripAt(initBufferRowIndex);
	
	TripDialog* dialog = new TripDialog(parent, mainWindow, db, multiEdit, std::move(originalTrip), bufferRowIndices.size());
	
	auto callWhenClosed = [=, &parent, &db]() {
		bool changesMade = false;
		
		if (dialog->result() == QDialog::Accepted && dialog->changesMade()) {
			unique_ptr<Trip> extractedTrip = dialog->extractData();
			extractedTrip->tripID = ItemID();
			QSet<const Column*> columnsToSave = dialog->getMultiEditColumns();
			QList<const Column*> columnList = QList<const Column*>(columnsToSave.constBegin(), columnsToSave.constEnd());
			
			db.beginChangingData();
			db.tripsTable.updateRows(parent, bufferRowIndices, columnList, *extractedTrip);
			db.finishChangingData();
			changesMade = true;
		}
		
		delete dialog;
		return callWhenDone(changesMade);
	};
	TripDialog::connect(dialog, &TripDialog::finished, callWhenClosed);
	
	dialog->open();
}

/**
 * Opens a delete trip dialog and deletes the trip from the database.
 *
 * @param parent			The parent window.
 * @param mainWindow		The application's main window.
 * @param db				The project database.
 * @param bufferRowIndices	The indices of the trips to delete in the database's trip table buffer.
 * @return					True if any items were deleted, false otherwise.
 */
bool openDeleteTripsDialogAndExecute(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices)
{
	Q_UNUSED(mainWindow);
	if (bufferRowIndices.isEmpty()) return false;
	
	QSet<ValidItemID> tripIDs = QSet<ValidItemID>();
	for (const BufferRowIndex& bufferRowIndex : bufferRowIndices) {
		tripIDs += VALID_ITEM_ID(db.tripsTable.primaryKeyColumn.getValueAt(bufferRowIndex));
	}
	
	QList<WhatIfDeleteResult> whatIfResults = db.whatIf_removeRows(db.tripsTable, tripIDs);
	
	if (Settings::confirmDelete.get()) {
		const QString windowTitle = TripDialog::tr("Delete %Ln trip(s)", "", tripIDs.size());
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return false;
	}
	
	db.beginChangingData();
	db.removeRows(parent, db.tripsTable, tripIDs);
	db.finishChangingData();
	return true;
}
