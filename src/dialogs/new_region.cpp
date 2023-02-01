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
	connect(cancelButton,		&QPushButton::clicked,	this,	&NewRegionDialog::handle_close);
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

void NewRegionDialog::handle_close()
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
	handle_close();
}



Region* openNewRegionDialogAndStore(QWidget *parent)
{
	NewRegionDialog dialog(parent);
	if (dialog.exec() == QDialog::Accepted) {
		QString name = dialog.nameTextbox->text();
		// TODO
		//return new Region(...);
	}
	return nullptr;
}

bool openEditRegionDialog(QWidget *parent, Region* region)
{
	// TODO
	return false;
}
