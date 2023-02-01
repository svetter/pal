#include "new_country.h"

#include <QMessageBox>



NewCountryDialog::NewCountryDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(cancelButton,	&QPushButton::clicked,	this,	&NewCountryDialog::reject);
}



bool NewCountryDialog::anyChanges()
{
	if (!nameTextbox->text().isEmpty())	return true;
	return false;
}



void NewCountryDialog::handle_close()
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
	handle_close();
}



Country* openNewCountryDialog(QWidget *parent)
{
	// TODO
	return nullptr;
}

bool openEditCountryDialog(QWidget *parent, Country* country)
{
	// TODO
	return false;
}
