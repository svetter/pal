#include "region_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"

#include <QMessageBox>



RegionDialog::RegionDialog(QWidget* parent, Database* db, DialogPurpose purpose, Region* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(newRangeButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_newRange);
	connect(newCountryButton,	&QPushButton::clicked,	this,	&RegionDialog::handle_newCountry);
	
	connect(okButton,			&QPushButton::clicked,	this,	&RegionDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,	this,	&RegionDialog::handle_cancel);
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	default:
		assert(false);
	}
}

RegionDialog::~RegionDialog()
{
	delete init;
}



QString RegionDialog::getEditWindowTitle()
{
	return tr("Edit region");
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
	// Name
	nameLineEdit->setText(init->name);
	// Range
	if (init->rangeID.isValid()) {
		rangeCombo->setCurrentIndex(db->rangesTable->getBufferIndexForPrimaryKey(init->rangeID.get()) + 1);	// 0 is None
	} else {
		rangeCombo->setCurrentIndex(0);
	}
	// Country
	if (init->countryID.isValid()) {
		countryCombo->setCurrentIndex(db->countriesTable->getBufferIndexForPrimaryKey(init->countryID.get()) + 1);	// 0 is None
	} else {
		countryCombo->setCurrentIndex(0);
	}
}


Region* RegionDialog::extractData()
{
	QString	name		= parseLineEdit	(nameLineEdit);
	ItemID	rangeID		= parseIDCombo	(rangeCombo);
	ItemID	countryID	= parseIDCombo	(countryCombo);
	
	Region* region = new Region(ItemID(), name, rangeID, countryID);
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





static int openRegionDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Region* originalRegion);

int openNewRegionDialogAndStore(QWidget* parent, Database* db)
{
	return openRegionDialogAndStore(parent, db, newItem, nullptr);
}

void openEditRegionDialogAndStore(QWidget* parent, Database* db, Region* originalRegion)
{
	openRegionDialogAndStore(parent, db, editItem, originalRegion);
}

void openDeleteRegionDialogAndExecute(QWidget* parent, Database* db, Region* region)
{
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->regionsTable, region->regionID.forceValid());
	
	QString windowTitle = RegionDialog::tr("Delete region");
	bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
	if (!proceed) return;

	db->removeRow(parent, db->regionsTable, region->regionID.forceValid());
}



static int openRegionDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Region* originalRegion)
{
	int newRegionIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalRegion);
		originalRegion->regionID = ItemID();
	}
	
	RegionDialog dialog(parent, db, purpose, originalRegion);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Region* extractedRegion = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newRegionIndex = db->regionsTable->addRow(parent, extractedRegion);
			break;
		case editItem:
			// TODO #107 update database
			break;
		default:
			assert(false);
		}
		
		delete extractedRegion;
	}
	
	return newRegionIndex;
}
