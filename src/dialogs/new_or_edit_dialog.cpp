#include "new_or_edit_dialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <iostream>



NewOrEditDialog::NewOrEditDialog(QWidget* parent, QString itemNameLowercase):
		QDialog(parent),
		itemNameLowercase(itemNameLowercase)
{}



bool NewOrEditDialog::changesMade()
{
	std::cerr << "bool NewOrEditDialog::changesMade() must be overwritten!" << std::endl;
	exit(EXIT_FAILURE);
}



void NewOrEditDialog::handle_cancel()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (changesMade()) {
		QString title = tr("Discard unsaved new ") + itemNameLowercase;
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewOrEditDialog::reject()
{
	handle_cancel();
}
