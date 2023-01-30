#include "new_country.h"

#include <QMessageBox>



NewCountryDialog::NewCountryDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewCountryDialog)
{
	ui->setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(ui->cancelButton,	&QPushButton::clicked,	this,	&NewCountryDialog::reject);
}

NewCountryDialog::~NewCountryDialog()
{
	delete ui;
}



bool NewCountryDialog::anyChanges()
{
	if (!ui->nameTextbox->text().isEmpty())	return true;
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
