#include "region_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"
#include "src/main/settings.h"

#include <QMessageBox>



RegionDialog::RegionDialog(QWidget* parent, Database* db, DialogPurpose purpose, Region* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init),
		selectableRangeIDs(QList<ValidItemID>())
{
	setupUi(this);
	
	restoreDialogGeometry(this, parent, &Settings::regionDialog_geometry);
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
	populateItemCombo(db->rangesTable, db->rangesTable->nameColumn, rangeCombo, selectableRangeIDs);
	
	populateItemCombo(db->countriesTable, db->countriesTable->nameColumn, countryCombo, selectableCountryIDs);
}



void RegionDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Range
	if (init->rangeID.isValid()) {
		rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(init->rangeID.get()) + 1);	// 0 is None
	} else {
		rangeCombo->setCurrentIndex(0);
	}
	// Country
	if (init->countryID.isValid()) {
		countryCombo->setCurrentIndex(selectableCountryIDs.indexOf(init->countryID.get()) + 1);	// 0 is None
	} else {
		countryCombo->setCurrentIndex(0);
	}
}


Region* RegionDialog::extractData()
{
	QString	name		= parseLineEdit		(nameLineEdit);
	ItemID	rangeID		= parseItemCombo	(rangeCombo, selectableRangeIDs);
	ItemID	countryID	= parseItemCombo	(countryCombo, selectableCountryIDs);
	
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
	int newRangeIndex = openNewRangeDialogAndStore(this, db);
	if (newRangeIndex < 0) return;
	
	populateItemCombo(db->rangesTable, db->rangesTable->nameColumn, rangeCombo, selectableRangeIDs);
	ValidItemID rangeID = db->rangesTable->getPrimaryKeyAt(newRangeIndex);
	rangeCombo->setCurrentIndex(selectableRangeIDs.indexOf(rangeID) + 1);	// 0 is None
}

void RegionDialog::handle_newCountry()
{
	int newCountryIndex = openNewCountryDialogAndStore(this, db);
	if (newCountryIndex < 0) return;
	
	populateItemCombo(db->countriesTable, db->countriesTable->nameColumn, countryCombo, selectableCountryIDs);
	ValidItemID countryID = db->countriesTable->getPrimaryKeyAt(newCountryIndex);
	countryCombo->setCurrentIndex(selectableRangeIDs.indexOf(countryID) + 1);	// 0 is None
}



void RegionDialog::handle_ok()
{
	aboutToClose();
	
	if (!nameLineEdit->text().isEmpty() || Settings::allowEmptyNames.get()) {
		accept();
	} else {
		QString title = tr("Can't save region");
		QString message = tr("The region needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

void RegionDialog::aboutToClose()
{
	saveDialogGeometry(this, parent, &Settings::regionDialog_geometry);
}





static int openRegionDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Region* originalRegion);

int openNewRegionDialogAndStore(QWidget* parent, Database* db)
{
	return openRegionDialogAndStore(parent, db, newItem, nullptr);
}

void openEditRegionDialogAndStore(QWidget* parent, Database* db, int bufferRowIndex)
{
	Region* originalRegion = db->getRegionAt(bufferRowIndex);
	openRegionDialogAndStore(parent, db, editItem, originalRegion);
}

void openDeleteRegionDialogAndExecute(QWidget* parent, Database* db, int bufferRowIndex)
{
	Region* region = db->getRegionAt(bufferRowIndex);
	ValidItemID regionID = region->regionID.forceValid();
	
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->regionsTable, regionID);
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = RegionDialog::tr("Delete region");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->regionsTable, regionID);
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
			db->regionsTable->updateRow(parent, originalRegion->regionID.forceValid(), extractedRegion);
			break;
		default:
			assert(false);
		}
		
		delete extractedRegion;
	}
	
	return newRegionIndex;
}
