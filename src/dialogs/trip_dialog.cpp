#include "trip_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



TripDialog::TripDialog(QWidget* parent, Database* db, DialogPurpose purpose, Trip* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	
	
	connect(datesUnspecifiedCheckbox,	&QCheckBox::stateChanged,	this,	&TripDialog::handle_datesSpecifiedChanged);	
	connect(startDateWidget,			&QDateEdit::dateChanged,	this,	&TripDialog::handle_startDateChanged);	
	connect(endDateWidget,				&QDateEdit::dateChanged,	this,	&TripDialog::handle_endDateChanged);
	
	connect(okButton,					&QPushButton::clicked,		this,	&TripDialog::handle_ok);
	connect(cancelButton,				&QPushButton::clicked,		this,	&TripDialog::handle_cancel);
	
	
	QDate initialDate = QDateTime::currentDateTime().date();
	startDateWidget->setDate(initialDate);
	endDateWidget->setDate(initialDate);
	
	
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
	if (!nameLineEdit->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save trip");
		QString message = tr("The trip needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}





static int openTripDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Trip* originalTrip);

int openNewTripDialogAndStore(QWidget* parent, Database* db)
{
	return openTripDialogAndStore(parent, db, newItem, nullptr);
}

void openEditTripDialogAndStore(QWidget* parent, Database* db, Trip* originalTrip)
{
	openTripDialogAndStore(parent, db, editItem, originalTrip);
}

void openDeleteTripDialogAndExecute(QWidget* parent, Database* db, Trip* trip)
{
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->tripsTable, trip->tripID.forceValid());
	
	QString windowTitle = TripDialog::tr("Delete Trip");
	bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
	if (!proceed) return;

	db->removeRow(parent, db->tripsTable, trip->tripID.forceValid());
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
			// TODO #107 update database
			break;
		default:
			assert(false);
		}
		
		delete extractedTrip;
	}
	
	return newTripIndex;
}
