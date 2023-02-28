#include "project_settings_window.h"

#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>



ProjectSettingsWindow::ProjectSettingsWindow(QWidget* parent, Database* db, bool firstOpen) :
		QDialog(parent),
		db(db),
		firstOpen(firstOpen)
{
	setupUi(this);
	
	if (firstOpen) {
		defaultHikerCombo->setVisible(false);
		newHikerButton->setVisible(false);
		bottomButtonBox->removeButton(bottomButtonBox->button(QDialogButtonBox::Apply));
	} else {
		newDefaultHikerLineEdit->setVisible(false);
	}
	
	const QRect savedGeometry = Settings::projectSettingsWindow_geometry.get();
	if (!savedGeometry.isEmpty()) {
		setGeometry(savedGeometry);
	}
	
	
	defaultHikerCombo->setModel(db->hikersTable);
	defaultHikerCombo->setRootModelIndex(db->hikersTable->getNullableRootModelIndex());
	defaultHikerCombo->setModelColumn(db->hikersTable->nameColumn->getIndex());
	
	
	connect(newHikerButton,				&QPushButton::clicked,		this,	&ProjectSettingsWindow::handle_newHiker);
	connect(usePhotosBasePathCheckbox,	&QCheckBox::stateChanged,	this,	&ProjectSettingsWindow::handle_photosBasePathCheckboxChanged);
	connect(photosBasePathBrowseButton,	&QPushButton::clicked,		this,	&ProjectSettingsWindow::handle_photosBasePathBrowse);
	
	connect(bottomButtonBox->button(QDialogButtonBox::Save),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_save);
	connect(bottomButtonBox->button(QDialogButtonBox::Apply),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_apply);
	connect(bottomButtonBox->button(QDialogButtonBox::Cancel),	&QPushButton::clicked,	this,	&ProjectSettingsWindow::handle_cancel);
	
	
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
	
	usePhotosBasePathCheckbox->setChecked(db->projectSettings->usePhotosBasePath->get());
	photosBasePathLineEdit->setText(db->projectSettings->photosBasePath->get());
	
	updateEnabled();
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
	
	if (photosBasePathLineEdit->text().isEmpty()) {
		usePhotosBasePathCheckbox->setChecked(false);
		handle_photosBasePathCheckboxChanged();
	}
	db->projectSettings->usePhotosBasePath->set(this, parseCheckbox(usePhotosBasePathCheckbox));
	db->projectSettings->photosBasePath->set(this, parseLineEdit(photosBasePathLineEdit));
}

void ProjectSettingsWindow::updateEnabled()
{
	handle_photosBasePathCheckboxChanged();
}



void ProjectSettingsWindow::handle_newHiker()
{
	int newHikerIndex = openNewHikerDialogAndStore(this, db);
	if (newHikerIndex < 0) return;
	defaultHikerCombo->setCurrentIndex(newHikerIndex + 1);	// 0 is None
}

void ProjectSettingsWindow::handle_photosBasePathCheckboxChanged()
{
	bool enabled = usePhotosBasePathCheckbox->checkState();
	photosBasePathLineEdit->setEnabled(enabled);
	photosBasePathBrowseButton->setEnabled(enabled);
}

void ProjectSettingsWindow::handle_photosBasePathBrowse()
{
	QString caption = tr("Select photos base folder");
	QString preSelectedDir = QString();
	QString basePath = QFileDialog::getExistingDirectory(this, caption, preSelectedDir);
	if (!basePath.isEmpty()) photosBasePathLineEdit->setText(basePath);
}



void ProjectSettingsWindow::handle_save()
{
	Settings::projectSettingsWindow_geometry.set(geometry());
	
	saveSettings();
}

void ProjectSettingsWindow::handle_apply()
{
	saveSettings();
}

void ProjectSettingsWindow::handle_cancel()
{
	Settings::projectSettingsWindow_geometry.set(geometry());
}



void ProjectSettingsWindow::reject()
{
	handle_cancel();
	QDialog::reject();
}