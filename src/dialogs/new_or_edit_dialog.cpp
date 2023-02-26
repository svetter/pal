#include "new_or_edit_dialog.h"

#include "src/main/settings.h"

#include <QPushButton>
#include <QMessageBox>



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
	aboutToClose();
	
	QMessageBox::StandardButton resultButton = QMessageBox::Yes;
	
	if (changesMade() && Settings::showCancelWarnings.get()) {
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





bool displayDeleteWarning(QWidget* parent, QString windowTitle, const QList<WhatIfDeleteResult>& whatIfResults)
{
	QString whatIfResultString = getTranslatedWhatIfDeleteResultDescription(whatIfResults);
	QString question = whatIfResultString + "\n" + NewOrEditDialog::tr("Are you sure?");
	
	QMessageBox::StandardButton resultButton = QMessageBox::Yes;
	auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
	auto selected = QMessageBox::Cancel;
	resultButton = QMessageBox::question(parent, windowTitle, question, options, selected);
	return resultButton == QMessageBox::Yes;
}
