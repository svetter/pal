#include "new_region.h"
#include "src/dialogs/new_range.h"
#include "src/dialogs/new_country.h"

#include <QMessageBox>



NewRegionDialog::NewRegionDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewRegionDialog)
{
	ui->setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(ui->newRangeButton,		&QPushButton::clicked,	this,	&NewRegionDialog::handle_newRange);	
	connect(ui->newCountryButton,	&QPushButton::clicked,	this,	&NewRegionDialog::handle_newCountry);	
	connect(ui->cancelButton,		&QPushButton::clicked,	this,	&NewRegionDialog::handle_close);
}

NewRegionDialog::~NewRegionDialog()
{
	delete ui;
}



bool NewRegionDialog::anyChanges()
{
	if (!ui->nameTextbox->text().isEmpty())		return true;
	if (ui->rangeCombo->currentIndex() > 0)		return true;
	if (ui->countryCombo->currentIndex() > 0)	return true;
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
