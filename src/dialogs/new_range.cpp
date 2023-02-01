#include "new_range.h"

#include <QMessageBox>



NewRangeDialog::NewRangeDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(cancelButton,	&QPushButton::clicked,	this,	&NewRangeDialog::handle_close);
}



bool NewRangeDialog::anyChanges()
{
	if (!nameTextbox->text().isEmpty())		return true;
	if (continentCombo->currentIndex() > 0)	return true;
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



Range* openNewRangeDialogAndStore(QWidget *parent)
{
	NewRangeDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		QString name = dialog.nameTextbox->text();
		// TODO
		//return new Range(...);
	}
	return nullptr;
}

bool openEditRangeDialog(QWidget *parent, Range* range)
{
	// TODO
	return false;
}
