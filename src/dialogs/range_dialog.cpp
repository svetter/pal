#include "range_dialog.h"
#include "src/dialogs/parse_helper.h"

#include <QMessageBox>



RangeDialog::RangeDialog(QWidget* parent, Database* db, Range* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(okButton,		&QPushButton::clicked,	this,	&RangeDialog::handle_ok);
	connect(cancelButton,	&QPushButton::clicked,	this,	&RangeDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}

RangeDialog::~RangeDialog()
{
	delete init;
}



void RangeDialog::populateComboBoxes()
{
	continentCombo->insertItems(1, Range::continentNames);
}



void RangeDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	nameLineEdit->setText(init->name);
	continentCombo->setCurrentIndex(init->continent);
}


Range* RangeDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	int		continent	= parseEnumCombo	(continentCombo);
	Range* range = new Range(-1, name, continent);
	return range;
}


bool RangeDialog::changesMade()
{
	Range* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void RangeDialog::handle_ok()
{
	if (!nameLineEdit->text().isEmpty()) {
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
		QString	name		= dialog.nameLineEdit->text();
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
