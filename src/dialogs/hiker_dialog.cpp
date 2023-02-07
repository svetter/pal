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

HikerDialog::~HikerDialog()
{
	delete init;
}



Hiker* HikerDialog::extractData()
{
	QString	name	= parseLineEdit	(nameLineEdit);
	Hiker* hiker = new Hiker(-1, name);
	return hiker;
}


void HikerDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	nameLineEdit->setText(init->name);
}


bool HikerDialog::changesMade()
{
	Hiker* currentState = extractData();
	bool equal = extractData()->equalTo(init);
	delete currentState;
	return !equal;
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
