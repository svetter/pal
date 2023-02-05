#include "new_or_edit_dialog.h"

#include "qpushbutton.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include <iostream>



NewOrEditDialog::NewOrEditDialog(QWidget* parent, Database* db, bool edit):
		QDialog(parent),
		db(db),
		edit(edit)
{}



void NewOrEditDialog::changeStringsForEdit(QPushButton* okButton)
{
	if (!edit) return;
	setWindowTitle(tr(("Edit " + itemNameLowercase).toLatin1()));
	okButton->setText(tr("Save changes"));
}



bool NewOrEditDialog::changesMade()
{
	std::cerr << "bool NewOrEditDialog::changesMade() must be overwritten!" << std::endl;
	exit(EXIT_FAILURE);
}



void NewOrEditDialog::handle_cancel()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (changesMade()) {
		QString title;
		if (edit) {
			title = tr("Discard changes");
		} else {
			title = tr("Discard new data");
		}
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
