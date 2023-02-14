#include "region_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"

#include <QMessageBox>



RegionDialog::RegionDialog(QWidget* parent, Database* db, Region* init) :
		NewOrEditDialog(parent, db, init != nullptr, tr("Edit region")),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(newRangeButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_newRange);
	connect(newCountryButton,	&QPushButton::clicked,	this,	&RegionDialog::handle_newCountry);
	
	connect(okButton,			&QPushButton::clicked,	this,	&RegionDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_cancel);
	
	
	if (edit) {	
		changeStringsForEdit(okButton);
		insertInitData();
	} else {
		this->init = extractData();
	}
}

RegionDialog::~RegionDialog()
{
	delete init;
}



void RegionDialog::populateComboBoxes()
{
	rangeCombo->setModel(db->rangesTable);
	rangeCombo->setRootModelIndex(db->rangesTable->getNullableRootModelIndex());
	rangeCombo->setModelColumn(db->rangesTable->nameColumn->getIndex());
	
	countryCombo->setModel(db->countriesTable);
	countryCombo->setRootModelIndex(db->countriesTable->getNullableRootModelIndex());
	countryCombo->setModelColumn(db->countriesTable->nameColumn->getIndex());
}



void RegionDialog::insertInitData()
{
	nameLineEdit->setText(init->name);
	rangeCombo->setCurrentIndex(db->rangesTable->getBufferIndexForPrimaryKey(init->rangeID));
	countryCombo->setCurrentIndex(db->countriesTable->getBufferIndexForPrimaryKey(init->countryID));
}


Region* RegionDialog::extractData()
{
	QString	name		= parseLineEdit	(nameLineEdit);
	int		rangeID		= parseIDCombo	(rangeCombo);
	int		countryID	= parseIDCombo	(countryCombo);
	Region* region = new Region(-1, name, rangeID, countryID);
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
	int newRegionIndex = openNewRangeDialogAndStore(this, db);
	rangeCombo->setCurrentIndex(newRegionIndex);
}

void RegionDialog::handle_newCountry()
{
	int newCountryIndex = openNewCountryDialogAndStore(this, db);
	countryCombo->setCurrentIndex(newCountryIndex);
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



int openNewRegionDialogAndStore(QWidget* parent, Database* db)
{
	int newRegionIndex = -1;
	
	RegionDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		Region* newRegion = dialog.extractData();
		newRegionIndex = db->regionsTable->addRow(parent, newRegion);
		delete newRegion;
	}
	
	return newRegionIndex;
}

void openEditRegionDialogAndStore(QWidget* parent, Database* db, Region* originalRegion)
{
	RegionDialog dialog(parent, db, originalRegion);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		Region* editedRegion = dialog.extractData();
		// TODO update database
		delete editedRegion;
	}
}
