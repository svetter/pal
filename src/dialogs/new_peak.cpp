#include "new_peak.h"
#include "src/dialogs/new_range.h"

#include <QMessageBox>
#include <iostream>



NewPeakDialog::NewPeakDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewPeakDialog)
{
	ui->setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(ui->cancelButton,	&QPushButton::clicked,	this,	&NewPeakDialog::handle_close);
}

NewPeakDialog::~NewPeakDialog()
{
	delete ui;
}



bool NewPeakDialog::anyChanges()
{
	if (!ui->nameTextbox->text().isEmpty())	return true;
	// TODO
	return false;
}



void NewPeakDialog::handle_newRange()
{
	NewRangeDialog dialog(this);
    dialog.exec();
}

void NewPeakDialog::reject()
{
    handle_close();
}

void NewPeakDialog::handle_close()
{
    QMessageBox::StandardButton resBtn = QMessageBox::Yes;
    if (anyChanges()) {
		QString title = tr("Discard unsaved new peak");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
        resBtn = QMessageBox::question(this, title, question, options, selected);
    }
    if (resBtn == QMessageBox::Yes) {
        QDialog::reject();
    }
}
