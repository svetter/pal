#include "trip_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



TripDialog::TripDialog(QWidget* parent, Database* db, Trip* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
{
	setupUi(this);
	
	
	connect(datesUnspecifiedCheckbox,	&QCheckBox::stateChanged,	this,	&TripDialog::handle_datesSpecifiedChanged);
	
	connect(okButton,					&QPushButton::clicked,		this,	&TripDialog::handle_ok);
	connect(cancelButton,				&QPushButton::clicked,		this,	&TripDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}

TripDialog::~TripDialog()
{
	delete init;
}



void TripDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	nameLineEdit->setText(init->name);
	startDateWidget->setDate(init->startDate);
	endDateWidget->setDate(init->endDate);
	descriptionEditor->setDocument(new QTextDocument(init->description));
}


Trip* TripDialog::extractData()
{
	QString	name		= parseLineEdit			(nameLineEdit);
	QDate	startDate	= parseDateWidget		(startDateWidget);
	QDate	endDate		= parseDateWidget		(endDateWidget);
	QString	description	= parsePlainTextEdit	(descriptionEditor);
	Trip* trip = new Trip(-1, name, startDate, endDate, description);
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
	startDateWidget->setEnabled(enabled);
	endDateWidget->setEnabled(enabled);
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



Trip* openNewTripDialogAndStore(QWidget* parent, Database* db)
{
	Trip* newTrip = nullptr;
	
	TripDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		newTrip = dialog.extractData();
		int tripID = db->tripsTable->addRow(newTrip);
		newTrip->tripID = tripID;
	}
	
	return newTrip;
}

Trip* openEditTripDialog(QWidget* parent, Database* db, Trip* originalTrip)
{
	Trip* editedTrip = nullptr;
	
	TripDialog dialog(parent, db, originalTrip);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		editedTrip = dialog.extractData();
		// TODO update database
	}
	
	delete originalTrip;
	return editedTrip;
}
