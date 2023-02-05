#include "region_dialog.h"

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



void RegionDialog::insertInitData()
{
	if (!init) return;
	// TODO
}



bool RegionDialog::changesMade()
{
	if (!nameTextbox->text().isEmpty())		return true;
	if (rangeCombo->currentIndex() > 0)		return true;
	if (countryCombo->currentIndex() > 0)	return true;
	return false;
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
	if (!nameTextbox->text().isEmpty()) {
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
		QString	name	= dialog.nameTextbox->text();
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
