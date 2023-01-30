#include "new_hiker.h"

#include <QMessageBox>



NewHikerDialog::NewHikerDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewHikerDialog)
{
	ui->setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(ui->cancelButton,	&QPushButton::clicked,	this,	&NewHikerDialog::handle_close);
}

NewHikerDialog::~NewHikerDialog()
{
	delete ui;
}



bool NewHikerDialog::anyChanges()
{
	if (!ui->nameTextbox->text().isEmpty())	return true;
	return false;
}



void NewHikerDialog::handle_close()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new hiker");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}



void NewHikerDialog::reject()
{
	handle_close();
}
