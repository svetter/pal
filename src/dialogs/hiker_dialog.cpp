#include "hiker_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



HikerDialog::HikerDialog(QWidget* parent, Database* db, Hiker* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(okButton,		&QPushButton::clicked,		this,	&HikerDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,		this,	&HikerDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}



void HikerDialog::insertInitData()
{
	if (!init) return;
	// TODO
}



bool HikerDialog::changesMade()
{
	if (!nameLineEdit->text().isEmpty())	return true;
	return false;
}



void HikerDialog::handle_ok()
{
	if (!nameLineEdit->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save hiker");
		QString message = tr("The hiker needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Hiker* openNewHikerDialogAndStore(QWidget* parent, Database* db)
{
	HikerDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		QString	name	= dialog.nameLineEdit->text();
		Hiker* hiker = new Hiker(-1, name);
		QList<QVariant> values = {QVariant(name)};
		db->addRow(db->hikersTable, values);
		return hiker;
	}
	return nullptr;
}

bool openEditHikerDialog(QWidget* parent, Database* db, Hiker* hiker)
{
	HikerDialog dialog(parent, db, hiker);
	dialog.exec();
	// TODO
	return false;
}
