#include "range_dialog.h"

#include <QMessageBox>



RangeDialog::RangeDialog(QWidget* parent): NewOrEditDialog(parent, tr("range"))
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
}



bool RangeDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())		return true;
	if (continentCombo->currentIndex() > 0)	return true;
	return false;
}



void RangeDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new mountain range");
		QString message = tr("The mountain range needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Range* openNewRangeDialogAndStore(QWidget* parent)
{
	RangeDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Range* range = new Range();
		range->name = dialog.nameTextbox->text();
		// TODO
		return range;
	}
	return nullptr;
}

bool openEditRangeDialog(QWidget* parent, Range* range)
{
	// TODO
	return false;
}
