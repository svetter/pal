#include "peak_dialog.h"

#include "src/dialogs/parse_helper.h"
#include "src/dialogs/region_dialog.h"

#include <QMessageBox>



PeakDialog::PeakDialog(QWidget* parent, Database* db, Peak* init) :
		NewOrEditDialog(parent, db, init != nullptr),
		init(init)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	populateComboBoxes();
	
	
	connect(heightCheckbox,		&QCheckBox::stateChanged,	this,	&PeakDialog::handle_heightSpecifiedChanged);
	connect(newRegionButton,	&QPushButton::clicked,		this,	&PeakDialog::handle_newRegion);
	
	connect(okButton,			&QPushButton::clicked,		this,	&PeakDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,		this,	&PeakDialog::handle_cancel);
	
	
	changeStringsForEdit(okButton);
	insertInitData();
}

PeakDialog::~PeakDialog()
{
	delete init;
}



void PeakDialog::populateComboBoxes()
{
	// TODO regionCombo
}



void PeakDialog::insertInitData()
{
	if (!edit) {
		init = extractData();
		return;
	}
	
	nameLineEdit->setText(init->name);
	heightSpinner->setValue(init->height);
	volcanoCheckbox->setChecked(init->volcano);
	// TODO regionCombo
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
	openNewRegionDialogAndStore(this, db);
	// TODO
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



Peak* openNewPeakDialogAndStore(QWidget* parent, Database* db)
{
	Peak* newPeak = nullptr;
	
	PeakDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted && dialog.changesMade()) {
		newPeak = dialog.extractData();
		int peakID = db->peaksTable->addRow(newPeak);
		newPeak->peakID = peakID;
	}
	
	return newPeak;
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
