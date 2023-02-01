#include "new_country.h"

#include <QMessageBox>



NewCountryDialog::NewCountryDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(okButton,		&QPushButton::clicked,	this,	&NewCountryDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&NewCountryDialog::handle_cancel);
}



bool NewCountryDialog::anyChanges()
{
	if (!nameTextbox->text().isEmpty())	return true;
	return false;
}



void NewCountryDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new country");
		QString question = tr("The country needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, question, ok, ok);
	}
}

void NewCountryDialog::handle_cancel()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new country");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewCountryDialog::reject()
{
	handle_cancel();
}



Country* openNewCountryDialogAndStore(QWidget *parent)
{
	NewCountryDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Country* country = new Country();
		country->name = dialog.nameTextbox->text();
		// TODO
		return country;
	}
	return nullptr;
}

bool openEditCountryDialog(QWidget *parent, Country* country)
{
	// TODO
	return false;
}
