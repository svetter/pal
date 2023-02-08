#include "region_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"

#include <QMessageBox>



RegionDialog::RegionDialog(QWidget* parent, Database* db, Region* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	
	connect(newRangeButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_newRange);
	connect(newCountryButton,	&QPushButton::clicked,	this,	&RegionDialog::handle_newCountry);
	
	connect(okButton,			&QPushButton::clicked,	this,	&RegionDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}

RegionDialog::~RegionDialog()
{
	delete init;
}



Region* RegionDialog::extractData()
{
	QString	name		= parseLineEdit	(nameLineEdit);
	int		countryID	= parseIDCombo	(countryCombo);
	Region* region = new Region(-1, name, countryID);
	return region;
}


void RegionDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	nameLineEdit->setText(init->name);
	// TODO countryCombo
}


bool RegionDialog::changesMade()
{
	Region* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void RegionDialog::handle_newRange()
{
	openNewRangeDialogAndStore(this, db);
	// TODO
}

void RegionDialog::handle_newCountry()
{
	openNewCountryDialogAndStore(this, db);
	// TODO
}



void RegionDialog::handle_ok()
{
	if (!nameLineEdit->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save region");
		QString message = tr("The region needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



Region* openNewRegionDialogAndStore(QWidget* parent, Database* db)
{
	RegionDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		QString	name	= dialog.nameLineEdit->text();
		int		rangeID	= -1;	// TODO
		Region* region = new Region(-1, name, rangeID);
		// TODO
		return region;
	}
	return nullptr;
}

bool openEditRegionDialog(QWidget* parent, Database* db, Region* region)
{
	RegionDialog dialog(parent, db, region);
	dialog.exec();
	// TODO
	return false;
}
