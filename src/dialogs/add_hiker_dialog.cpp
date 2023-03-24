#include "add_hiker_dialog.h"

#include "src/dialogs/hiker_dialog.h"

#include <QMessageBox>



AddHikerDialog::AddHikerDialog(QWidget* parent, Database* db) :
		QDialog(parent),
		db(db),
		selectableHikerIDs(QList<ValidItemID>())
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(newHikerButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_newHiker);
	
	connect(okButton,		&QPushButton::clicked,	this,	&AddHikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&AddHikerDialog::handle_cancel);
}



void AddHikerDialog::populateComboBoxes()
{
	populateItemCombo(db->hikersTable, db->hikersTable->nameColumn, true, hikerCombo, selectableHikerIDs);
}



ValidItemID AddHikerDialog::extractHikerID()
{
	assert(hikerSelected());
	return selectableHikerIDs.at(hikerCombo->currentIndex() - 1);
}


bool AddHikerDialog::hikerSelected()
{
	return hikerCombo->currentIndex() > 0;
}



void AddHikerDialog::handle_newHiker()
{
	int newHikerIndex = openNewHikerDialogAndStore(this, db);
	if (newHikerIndex >= 0) {
		hikerCombo->setCurrentIndex(newHikerIndex + 1);	// 0 is None
	}
}


void AddHikerDialog::handle_ok()
{
	if (hikerSelected()) {
		accept();
	} else {
		QString title = tr("Can't add hiker");
		QString message = tr("You have to select a hiker.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

void AddHikerDialog::handle_cancel()
{
	reject();
}



ItemID openAddHikerDialog(QWidget* parent, Database* db)
{
	AddHikerDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		return dialog.extractHikerID();
	} else {
		return -1;
	}
}
