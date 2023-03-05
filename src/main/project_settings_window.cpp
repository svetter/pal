#include "project_settings_window.h"

#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>



ProjectSettingsWindow::ProjectSettingsWindow(QWidget* parent, Database* db, bool firstOpen) :
		QDialog(parent),
		parent(parent),
		db(db),
		firstOpen(firstOpen)
{
	setupUi(this);
	setFixedSize(sizeHint());
	
	if (firstOpen) {
		defaultHikerCombo->setVisible(false);
		newHikerButton->setVisible(false);
		bottomButtonBox->removeButton(bottomButtonBox->button(QDialogButtonBox::Apply));
	} else {
		newDefaultHikerLineEdit->setVisible(false);
	}
	
	restoreDialogGeometry(this, parent, &Settings::projectSettingsWindow_geometry);
	
	
	defaultHikerCombo->setModel(db->hikersTable);
	defaultHikerCombo->setRootModelIndex(db->hikersTable->getNullableRootModelIndex());
	defaultHikerCombo->setModelColumn(db->hikersTable->nameColumn->getIndex());
	
	
	connect(newHikerButton,										&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_newHiker);
	
	connect(bottomButtonBox->button(QDialogButtonBox::Save),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_save);
	connect(bottomButtonBox->button(QDialogButtonBox::Cancel),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_cancel);
	if (!firstOpen)
	connect(bottomButtonBox->button(QDialogButtonBox::Apply),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_apply);
	
	
	loadSettings();
}



void ProjectSettingsWindow::loadSettings()
{
	ItemID hikerID = db->projectSettings->defaultHiker->get();
	if (hikerID.isValid()) {
		int bufferRowIndex = db->hikersTable->getBufferIndexForPrimaryKey(hikerID.get());
		defaultHikerCombo->setCurrentIndex(bufferRowIndex + 1);	// 0 is None
	} else {
		defaultHikerCombo->setCurrentIndex(0);
	}
}

void ProjectSettingsWindow::saveSettings()
{
	if (firstOpen && !newDefaultHikerLineEdit->text().isEmpty()) {
		QString newDefaultHikerName = newDefaultHikerLineEdit->text();
		Hiker* newDefaultHiker = new Hiker(ItemID(), newDefaultHikerName);
		int newHikerIndex = db->hikersTable->addRow(this, newDefaultHiker);
		defaultHikerCombo->setCurrentIndex(newHikerIndex + 1);	// 0 is None
	}
	db->projectSettings->defaultHiker->set(this, parseIDCombo(defaultHikerCombo).asQVariant());
}



void ProjectSettingsWindow::handle_newHiker()
{
	int newHikerIndex = openNewHikerDialogAndStore(this, db);
	if (newHikerIndex < 0) return;
	defaultHikerCombo->setCurrentIndex(newHikerIndex + 1);	// 0 is None
}



void ProjectSettingsWindow::handle_save()
{
	saveSettings();
	Settings::projectSettingsWindow_geometry.set(geometry());
	accept();
}

void ProjectSettingsWindow::handle_apply()
{
	saveSettings();
}

void ProjectSettingsWindow::handle_cancel()
{
	saveDialogGeometry(this, parent, &Settings::projectSettingsWindow_geometry);
	QDialog::reject();
}



void ProjectSettingsWindow::reject()
{
	handle_cancel();
}