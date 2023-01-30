#include "new_range.h"

#include <QMessageBox>



NewRangeDialog::NewRangeDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewRangeDialog)
{
	ui->setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(ui->cancelButton,	&QPushButton::clicked,	this,	&NewRangeDialog::handle_close);
}

NewRangeDialog::~NewRangeDialog()
{
	delete ui;
}



bool NewRangeDialog::anyChanges()
{
	if (!ui->nameTextbox->text().isEmpty())		return true;
	if (ui->continentCombo->currentIndex() > 0)	return true;
	return false;
}



void NewRangeDialog::handle_close()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new range");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}



void NewRangeDialog::reject()
{
	handle_close();
}
