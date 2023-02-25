#include "add_hiker_dialog.h"

#include "src/dialogs/hiker_dialog.h"

#include <QMessageBox>



AddHikerDialog::AddHikerDialog(QWidget* parent, Database* db) :
		QDialog(parent),
		db(db)
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
	hikerCombo->setModel(db->hikersTable);
	hikerCombo->setRootModelIndex(db->hikersTable->getNullableRootModelIndex());
	hikerCombo->setModelColumn(db->hikersTable->nameColumn->getIndex());
}



int AddHikerDialog::extractHikerIndex()
{
	return hikerCombo->currentIndex();
}


bool AddHikerDialog::hikerSelected()
{
	return extractHikerIndex() >= 0;
}



void AddHikerDialog::handle_newHiker()
{
	int newHikerIndex = openNewHikerDialogAndStore(this, db);
	if (newHikerIndex >= 0) {
		hikerCombo->setCurrentIndex(newHikerIndex);
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



int openAddHikerDialog(QWidget* parent, Database* db)
{
	AddHikerDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		return dialog.extractHikerIndex();
	} else {
		return -1;
	}
}
