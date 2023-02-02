#include "range_dialog.h"

#include <QMessageBox>



RangeDialog::RangeDialog(QWidget* parent, Range* init) :
		NewOrEditDialog(parent, tr("range"), init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}



void RangeDialog::insertInitData()
{
	if (!init) return;
	// TODO
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
	RangeDialog dialog(parent, range);
	dialog.exec();
	// TODO
	return false;
}
