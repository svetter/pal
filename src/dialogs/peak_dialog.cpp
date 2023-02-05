#include "peak_dialog.h"

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



void PeakDialog::insertInitData()
{
	if (!init) return;
	// TODO
}



bool PeakDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())			return true;
	if (regionCombo->currentIndex() > 0)		return true;
	if (volcanoCheckbox->isChecked())			return true;
	if (!googleMapsTextbox->text().isEmpty())	return true;
	if (!googleEarthTextbox->text().isEmpty())	return true;
	if (!wikipediaTextbox->text().isEmpty())	return true;
	return false;
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
	if (!nameTextbox->text().isEmpty()) {
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
		QString	name		= dialog.nameTextbox->text();
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
