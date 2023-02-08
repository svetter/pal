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



Trip* TripDialog::extractData()
{
	QString	name		= parseLineEdit			(nameLineEdit);
	QDate	startDate	= parseDateWidget		(startDateWidget);
	QDate	endDate		= parseDateWidget		(endDateWidget);
	QString	description	= parsePlainTextEdit	(descriptionEditor);
	Trip* trip = new Trip(-1, name, startDate, endDate, description);
	return trip;
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
	TripDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		QString	name		= dialog.nameLineEdit->text();
		QDate	startDate	= QDate();	// TODO
		QDate	endDate		= QDate();	// TODO
		QString	notes		= "";	// TODO
		Trip* trip = new Trip(-1, name, startDate, endDate, notes);
		// TODO
		return trip;
	}
	return nullptr;
}

bool openEditTripDialog(QWidget* parent, Database* db, Trip* trip)
{
	TripDialog dialog(parent, db, trip);
	dialog.exec();
	// TODO
	return false;
}
