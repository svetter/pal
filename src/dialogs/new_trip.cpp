#include "new_trip.h"

#include <QMessageBox>



NewTripDialog::NewTripDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewTripDialog)
{
	ui->setupUi(this);
	
	connect(ui->cancelButton,	&QPushButton::clicked,	this,	&NewTripDialog::handle_close);
}

NewTripDialog::~NewTripDialog()
{
	delete ui;
}



bool NewTripDialog::anyChanges()
{
	if (!ui->nameTextbox->text().isEmpty())				return true;
	if (!ui->descriptionEditor->document()->isEmpty())	return true;
	return false;
}



void NewTripDialog::handle_close()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new trip");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}



void NewTripDialog::reject()
{
	handle_close();
}
