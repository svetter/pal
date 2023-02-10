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
	
	populateComboBoxes();
	
	
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



void RegionDialog::populateComboBoxes()
{
	// TODO #96 rangeCombo
	// TODO #96 countryCombo
}



void RegionDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	nameLineEdit->setText(init->name);
	// TODO #96 rangeCombo
	// TODO #96 countryCombo
}


Region* RegionDialog::extractData()
{
	QString	name		= parseLineEdit	(nameLineEdit);
	int		countryID	= parseIDCombo	(countryCombo);
	Region* region = new Region(-1, name, countryID);
	return region;
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
	Range* newRange = openNewRangeDialogAndStore(this, db);
	if (!newRange) return;
	int rangeID = newRange->rangeID;
	QString& name = newRange->name;
	// TODO #96 add to rangeCombo
}

void RegionDialog::handle_newCountry()
{
	Country* newCountry = openNewCountryDialogAndStore(this, db);
	if (!newCountry) return;
	int countryID = newCountry->countryID;
	QString& name = newCountry->name;
	// TODO #96 add to countryCombo
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
	Region* newRegion = nullptr;
	
	RegionDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		newRegion = dialog.extractData();
		int regionID = db->regionsTable->addRow(newRegion);
		newRegion->regionID = regionID;
	}
	
	return newRegion;
}

Region* openEditRegionDialog(QWidget* parent, Database* db, Region* originalRegion)
{
	Region* editedRegion = nullptr;
	
	RegionDialog dialog(parent, db, originalRegion);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		editedRegion = dialog.extractData();
		// TODO update database
	}
	
	delete originalRegion;
	return editedRegion;
}
