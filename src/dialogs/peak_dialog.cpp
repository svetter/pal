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


bool PeakDialog::changesMade()
{
	Peak* currentState = extractData();
	bool equal = extractData()->equalTo(init);
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
	PeakDialog dialog(parent, db);
	if (dialog.exec() == QDialog::Accepted) {
		QString	name		= dialog.nameLineEdit->text();
		int		height		= -1;	// TODO
		bool	volcano		= false;	// TODO
		int		regionID	= -1;	// TODO
		QString	mapsLink	= "";	// TODO
		QString	earthLink	= "";	// TODO
		QString	wikiLink	= "";	// TODO
		Peak* peak = new Peak(-1, name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
		// TODO
		return peak;
	}
	return nullptr;
}

bool openEditPeakDialog(QWidget* parent, Database* db, Peak* peak)
{
	PeakDialog dialog(parent, db, peak);
	dialog.exec();
	// TODO
	return false;
}
