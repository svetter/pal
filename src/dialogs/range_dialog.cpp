#include "range_dialog.h"

#include <QMessageBox>



RangeDialog::RangeDialog(QWidget* parent, Database* db, Range* init) :
		NewOrEditDialog(parent, db, init != nullptr),
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
		QString title = tr("Can't save mountain range");
		QString message = tr("The mountain range needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Range* openNewRangeDialogAndStore(QWidget* parent, Database* db)
{
	RangeDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		QString	name		= dialog.nameTextbox->text();
		int		continent	= -1;	// TODO
		Range* range = new Range(-1, name, continent);
		// TODO
		return range;
	}
	return nullptr;
}

bool openEditRangeDialog(QWidget* parent, Database* db, Range* range)
{
	RangeDialog dialog(parent, db, range);
	dialog.exec();
	// TODO
	return false;
}
