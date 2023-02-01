#include "new_range.h"

#include <QMessageBox>



NewRangeDialog::NewRangeDialog(QWidget *parent): NewOrEditDialog(parent, tr("range"))
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(okButton,		&QPushButton::clicked,	this,	&NewRangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&NewRangeDialog::handle_cancel);
}



bool NewRangeDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())		return true;
	if (continentCombo->currentIndex() > 0)	return true;
	return false;
}



void NewRangeDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new mountain range");
		QString question = tr("The mountain range needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, question, ok, ok);
	}
}



Range* openNewRangeDialogAndStore(QWidget *parent)
{
	NewRangeDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Range* range = new Range();
		range->name = dialog.nameTextbox->text();
		// TODO
		return range;
	}
	return nullptr;
}

bool openEditRangeDialog(QWidget *parent, Range* range)
{
	// TODO
	return false;
}
