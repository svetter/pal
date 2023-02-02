#include "region_dialog.h"

#include "src/dialogs/range_dialog.h"
#include "src/dialogs/country_dialog.h"

#include <QMessageBox>



RegionDialog::RegionDialog(QWidget* parent, Region* init) :
		NewOrEditDialog(parent, init != nullptr),
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
	openNewRangeDialogAndStore(this);
	// TODO
}

void RegionDialog::handle_newCountry()
{
	Country c = Country();
	c.name = "Uzbekistan";
	openNewCountryDialogAndStore(this);
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



Region* openNewRegionDialogAndStore(QWidget* parent)
{
	RegionDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Region* region = new Region();
		region->name = dialog.nameTextbox->text();
		// TODO
		return region;
	}
	return nullptr;
}

bool openEditRegionDialog(QWidget* parent, Region* region)
{
	RegionDialog dialog(parent, region);
	dialog.exec();
	// TODO
	return false;
}
