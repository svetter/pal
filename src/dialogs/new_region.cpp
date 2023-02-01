#include "new_region.h"
#include "src/dialogs/new_range.h"
#include "src/dialogs/new_country.h"

#include <QMessageBox>



NewRegionDialog::NewRegionDialog(QWidget *parent): QDialog(parent)
{
	setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(newRangeButton,		&QPushButton::clicked,	this,	&NewRegionDialog::handle_newRange);
	connect(newCountryButton,	&QPushButton::clicked,	this,	&NewRegionDialog::handle_newCountry);
	
	connect(okButton,			&QPushButton::clicked,	this,	&NewRegionDialog::handle_ok);
	connect(cancelButton,		&QPushButton::clicked,	this,	&NewRegionDialog::handle_cancel);
}



bool NewRegionDialog::anyChanges()
{
	if (!nameTextbox->text().isEmpty())		return true;
	if (rangeCombo->currentIndex() > 0)		return true;
	if (countryCombo->currentIndex() > 0)	return true;
	return false;
}



void NewRegionDialog::handle_newRange()
{
	NewRangeDialog dialog(this);
	dialog.exec();
}

void NewRegionDialog::handle_newCountry()
{
	NewCountryDialog dialog(this);
	dialog.exec();
}



void NewRegionDialog::handle_ok()
{
	if (!nameTextbox->text().isEmpty()) {
		accept();
	} else {
		QString title = tr("Can't save new region");
		QString question = tr("The region needs a name.");
		auto ok = QMessageBox::Ok;
		QMessageBox::information(this, title, question, ok, ok);
	}
}

void NewRegionDialog::handle_cancel()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new region");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}


void NewRegionDialog::reject()
{
	handle_cancel();
}



Region* openNewRegionDialogAndStore(QWidget *parent)
{
	NewRegionDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		Region* region = new Region();
		region->name = dialog.nameTextbox->text();
		// TODO
		return region;
	}
	return nullptr;
}

bool openEditRegionDialog(QWidget *parent, Region* region)
{
	// TODO
	return false;
}
