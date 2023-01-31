#include "new_peak.h"
#include "src/dialogs/new_region.h"

#include <QMessageBox>



NewPeakDialog::NewPeakDialog(QWidget *parent): QDialog(parent), ui(new Ui::NewPeakDialog)
{
	ui->setupUi(this);
	setFixedHeight(minimumSizeHint().height());
	
	connect(ui->heightCheckbox,		&QCheckBox::stateChanged,	this,	&NewPeakDialog::handle_heightSpecifiedChanged);
	connect(ui->newRegionButton,	&QPushButton::clicked,		this,	&NewPeakDialog::handle_newRegion);
	connect(ui->cancelButton,		&QPushButton::clicked,		this,	&NewPeakDialog::handle_close);	
}

NewPeakDialog::~NewPeakDialog()
{
	delete ui;
}



bool NewPeakDialog::anyChanges()
{
	if (!ui->nameTextbox->text().isEmpty())			return true;
	if (ui->regionCombo->currentIndex() > 0)		return true;
	if (ui->volcanoCheckbox->isChecked())			return true;
	if (!ui->googleMapsTextbox->text().isEmpty())	return true;
	if (!ui->googleEarthTextbox->text().isEmpty())	return true;
	if (!ui->wikipediaTextbox->text().isEmpty())	return true;
	return false;
}



void NewPeakDialog::handle_heightSpecifiedChanged()
{
	bool enabled = ui->heightCheckbox->isChecked();
	ui->heightSpinner->setEnabled(enabled);
}

void NewPeakDialog::handle_newRegion()
{
	NewRegionDialog dialog(this);
	dialog.exec();
}

void NewPeakDialog::handle_close()
{
	QMessageBox::StandardButton resBtn = QMessageBox::Yes;
	if (anyChanges()) {
		QString title = tr("Discard unsaved new peak");
		QString question = tr("Are you sure?");
		auto options = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
		auto selected = QMessageBox::Cancel;
		resBtn = QMessageBox::question(this, title, question, options, selected);
	}
	if (resBtn == QMessageBox::Yes) {
		QDialog::reject();
	}
}



void NewPeakDialog::reject()
{
	handle_close();
}
