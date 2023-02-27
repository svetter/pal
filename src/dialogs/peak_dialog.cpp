#include "peak_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/region_dialog.h"
#include "src/main/settings.h"

#include <QMessageBox>



PeakDialog::PeakDialog(QWidget* parent, Database* db, DialogPurpose purpose, Peak* init) :
		NewOrEditDialog(parent, db, purpose),
		init(init)
{
	setupUi(this);
	
	const QRect savedGeometry = Settings::peakDialog_geometry.get();
	if (!savedGeometry.isEmpty()) {
		setGeometry(savedGeometry);
	}
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(heightCheckbox,		&QCheckBox::stateChanged,	this,	&PeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,	&QPushButton::clicked,		this,	&PeakDialog::handle_newRegion);
	
	connect(okButton,			&QPushButton::clicked,		this,	&PeakDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,		this,	&PeakDialog::handle_cancel);
	
	
	// Set initial height
	heightCheckbox->setChecked(Settings::peakDialog_heightEnabledInitially.get());
	handle_heightSpecifiedChanged();
	heightSpinner->setValue(Settings::peakDialog_initialHeight.get());
	
	
	switch (purpose) {
	case newItem:
		this->init = extractData();
		break;
	case editItem:
		changeStringsForEdit(okButton);
		insertInitData();
		break;
	case duplicateItem:
		Peak* blankPeak = extractData();
		insertInitData();
		this->init = blankPeak;
		break;
	}
}

PeakDialog::~PeakDialog()
{
	delete init;
}



QString PeakDialog::getEditWindowTitle()
{
	return tr("Edit peak");
}



void PeakDialog::populateComboBoxes()
{
	regionCombo->setModel(db->regionsTable);
	regionCombo->setRootModelIndex(db->regionsTable->getNullableRootModelIndex());
	regionCombo->setModelColumn(db->regionsTable->nameColumn->getIndex());
}



void PeakDialog::insertInitData()
{
	// Name
	nameLineEdit->setText(init->name);
	// Height
	bool heightSpecified = init->heightSpecified();
	heightCheckbox->setChecked(heightSpecified);
	if (heightSpecified) {
		heightSpinner->setValue(init->height);
	}	
	handle_heightSpecifiedChanged();
	// Volcano
	volcanoCheckbox->setChecked(init->volcano);
	// Region
	if (init->regionID.isValid()) {
		regionCombo->setCurrentIndex(db->regionsTable->getBufferIndexForPrimaryKey(init->regionID.get()) + 1);	// 0 is None
	} else {
		regionCombo->setCurrentIndex(0);
	}
	// Links
	googleMapsLineEdit->setText(init->mapsLink);
	googleEarthLineEdit->setText(init->earthLink);
	wikipediaLineEdit->setText(init->wikiLink);
}


Peak* PeakDialog::extractData()
{
	QString	name		= parseLineEdit	(nameLineEdit);
	int		height		= parseSpinner	(heightSpinner);
	bool	volcano		= parseCheckbox	(volcanoCheckbox);
	ItemID	regionID	= parseIDCombo	(regionCombo);
	QString	mapsLink	= parseLineEdit	(googleMapsLineEdit);
	QString	earthLink	= parseLineEdit	(googleEarthLineEdit);
	QString	wikiLink	= parseLineEdit	(wikipediaLineEdit);
	
	if (!heightCheckbox->isChecked())	height = -1;
	
	Peak* peak = new Peak(ItemID(), name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
	return peak;
}


bool PeakDialog::changesMade()
{
	Peak* currentState = extractData();
	bool equal = currentState->equalTo(init);
	delete currentState;
	return !equal;
}



void PeakDialog::handle_heightSpecifiedChanged()
{
	bool enabled = heightCheckbox->isChecked();
	heightSpinner->setEnabled(enabled);
}

void PeakDialog::handle_newRegion()
{
	int newRegionIndex = openNewRegionDialogAndStore(this, db);
	regionCombo->setCurrentIndex(newRegionIndex + 1);	// 0 is None
}



void PeakDialog::handle_ok()
{
	aboutToClose();
	
	if (!nameLineEdit->text().isEmpty() || Settings::allowEmptyNames.get()) {
		accept();
	} else {
		QString title = tr("Can't save peak");
		QString message = tr("The peak needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}

void PeakDialog::aboutToClose()
{	
	Settings::peakDialog_geometry.set(geometry());
}





static int openPeakDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Peak* originalPeak);

int openNewPeakDialogAndStore(QWidget* parent, Database* db)
{
	return openPeakDialogAndStore(parent, db, newItem, nullptr);
}

int openDuplicatePeakDialogAndStore(QWidget* parent, Database* db, Peak* copyFrom)
{
	return openPeakDialogAndStore(parent, db, duplicateItem, copyFrom);
}

void openEditPeakDialogAndStore(QWidget* parent, Database* db, Peak* originalPeak)
{
	openPeakDialogAndStore(parent, db, editItem, originalPeak);
}

void openDeletePeakDialogAndExecute(QWidget* parent, Database* db, Peak* peak)
{
	QList<WhatIfDeleteResult> whatIfResults = db->whatIf_removeRow(db->peaksTable, peak->peakID.forceValid());
	
	if (Settings::confirmDelete.get()) {
		QString windowTitle = PeakDialog::tr("Delete peak");
		bool proceed = displayDeleteWarning(parent, windowTitle, whatIfResults);
		if (!proceed) return;
	}

	db->removeRow(parent, db->peaksTable, peak->peakID.forceValid());
}



static int openPeakDialogAndStore(QWidget* parent, Database* db, DialogPurpose purpose, Peak* originalPeak)
{
	int newPeakIndex = -1;
	if (purpose == duplicateItem) {
		assert(originalPeak);
		originalPeak->peakID = ItemID();
	}
	
	PeakDialog dialog(parent, db, purpose, originalPeak);
	if (dialog.exec() == QDialog::Accepted && (purpose != editItem || dialog.changesMade())) {
		Peak* extractedPeak = dialog.extractData();
		
		switch (purpose) {
		case newItem:
		case duplicateItem:
			newPeakIndex = db->peaksTable->addRow(parent, extractedPeak);
			break;
		case editItem:
			db->peaksTable->updateRow(parent, originalPeak->peakID.forceValid(), extractedPeak);
			break;
		default:
			assert(false);
		}
		
		delete extractedPeak;
	}
	
	return newPeakIndex;
}
