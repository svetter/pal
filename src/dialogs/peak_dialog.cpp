#include "peak_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/region_dialog.h"

#include <QMessageBox>



PeakDialog::PeakDialog(QWidget* parent, Database* db, Peak* init) :
		NewOrEditDialog(parent, db, init != nullptr, tr("Edit peak")),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(heightCheckbox,		&QCheckBox::stateChanged,	this,	&PeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,	&QPushButton::clicked,		this,	&PeakDialog::handle_newRegion);
	
	connect(okButton,			&QPushButton::clicked,		this,	&PeakDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,		this,	&PeakDialog::handle_cancel);
	
	
	if (edit) {	
		changeStringsForEdit(okButton);
		insertInitData();
	} else {
		this->init = extractData();
	}
}

PeakDialog::~PeakDialog()
{
	delete init;
}



void PeakDialog::populateComboBoxes()
{
	regionCombo->setModel(db->regionsTable);
	regionCombo->setRootModelIndex(db->regionsTable->getNullableRootModelIndex());
	regionCombo->setModelColumn(db->regionsTable->nameColumn->getIndex());
}



void PeakDialog::insertInitData()
{
	nameLineEdit->setText(init->name);
	heightSpinner->setValue(init->height);
	volcanoCheckbox->setChecked(init->volcano);
	regionCombo->setCurrentIndex(db->regionsTable->getBufferIndexForPrimaryKey(init->regionID));
	googleMapsLineEdit->setText(init->mapsLink);
	googleEarthLineEdit->setText(init->earthLink);
	wikipediaLineEdit->setText(init->wikiLink);
}


Peak* PeakDialog::extractData()
{
	QString	name		= parseLineEdit	(nameLineEdit);
	int		height		= parseSpinner	(heightSpinner);
	bool	volcano		= parseCheckbox	(volcanoCheckbox);
	int		regionID	= parseIDCombo	(regionCombo);
	QString	mapsLink	= parseLineEdit	(googleMapsLineEdit);
	QString	earthLink	= parseLineEdit	(googleEarthLineEdit);
	QString	wikiLink	= parseLineEdit	(wikipediaLineEdit);
	Peak* peak = new Peak(-1, name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
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
	regionCombo->setCurrentIndex(newRegionIndex);
}



void PeakDialog::handle_ok()
{
	if (!nameLineEdit->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save peak");
		QString message = tr("The peak needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, message, ok, ok);
	}
}



int openNewPeakDialogAndStore(QWidget* parent, Database* db)
{
	int newPeakIndex = -1;
	
	PeakDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		Peak* newPeak = dialog.extractData();
		newPeakIndex = db->peaksTable->addRow(newPeak);
		delete newPeak;
	}
	
	return newPeakIndex;
}

Peak* openEditPeakDialog(QWidget* parent, Database* db, Peak* originalPeak)
{
	Peak* editedPeak = nullptr;
	
	PeakDialog dialog(parent, db, originalPeak);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		editedPeak = dialog.extractData();
		// TODO update database
	}
	
	delete originalPeak;
	return editedPeak;
}
