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

#include "trip_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QMessageBox>



TripDialog::TripDialog(QWidget* parent, Database* db, DialogPurpose purpose, Trip* init) :
		NewOrEditDialog(parent, db, purpose),
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

TripDialog::~TripDialog()
{
	delete init;
}



QString TripDialog::getEditWindowTitle()
{
	return tr("Edit trip");
}



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


bool TripDialog::changesMade()
{
	Trip* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void TripDialog::handle_datesSpecifiedChanged()
{
	bool enabled = !datesUnspecifiedCheckbox->isChecked();
	startDateLabel->setEnabled(enabled);
	endDateLabel->setEnabled(enabled);
	startDateWidget->setEnabled(enabled);
	endDateWidget->setEnabled(enabled);
}


void TripDialog::handle_startDateChanged()
{
	if (endDateWidget->date() < startDateWidget->date()) {
		endDateWidget->setDate(startDateWidget->date());
	}
}

void TripDialog::handle_endDateChanged()
{
	if (startDateWidget->date() > endDateWidget->date()) {
		startDateWidget->setDate(endDateWidget->date());
	}
}



void TripDialog::handle_ok()
{
	QString emptyNameWindowTitle	= tr("Can't save trip");
	QString emptyNameWindowMessage	= tr("The trip needs a name.");
	const Column* nameColumn = db->tripsTable->nameColumn;
	NewOrEditDialog::handle_ok(nameLineEdit, init->name, emptyNameWindowTitle, emptyNameWindowMessage, nameColumn);
}

void TripDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::tripDialog_geometry);
}





static int openTripDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Trip* originalTrip);

int openNewTripDialogAndStore(QWidget* parent, Database* db)
{
	return openTripDialogAndStore(parent, db, newItem, nullptr);
}

void openEditTripDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Trip* originalTrip = db->getTripAt(bufferRowIndex);
	openTripDialogAndStore(parent, db, editItem, originalTrip);
}

void openDeleteTripDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex)
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



static int openTripDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Trip* originalTrip)
{
	int newTripIndex = -1;
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
