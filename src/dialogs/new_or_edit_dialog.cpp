#include "new_or_edit_dialog.h"

#include "qpushbutton.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>



NewOrEditDialog::NewOrEditDialog(QWidget* parent, Database* db, DialogPurpose purpose):
		QDialog(parent),
		db(db),
		purpose(purpose)
{}



void NewOrEditDialog::changeStringsForEdit(QPushButton* okButton)
{
	if (purpose != editItem) return;
	setWindowTitle(getEditWindowTitle());
	okButton->setText(tr("Save changes"));
}



void NewOrEditDialog::handle_cancel()
{
	QMessageBox::StandardButton resultButton = QMessageBox::Yes;
	if (changesMade()) {
		QString title;
		if (purpose != editItem) {
			title = tr("Discard changes");
		} else {
			title = tr("Discard unsaved data");
		}
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resultButton = QMessageBox::question(this, title, question, options, selected);
	}
	if (resultButton == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewOrEditDialog::reject()
{
	handle_cancel();
}
