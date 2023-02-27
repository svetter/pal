#include "project_settings_window.h"

#include "src/dialogs/hiker_dialog.h"
#include "src/dialogs/parse_helper.h"
#include "src/main/settings.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>



ProjectSettingsWindow::ProjectSettingsWindow(QWidget* parent, Database* db) :
		QDialog(parent),
		db(db)
{
	setupUi(this);
	
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
	ItemID hikerID = db->projectSettings->getValue(db->projectSettings->defaultHiker).toInt();
	if (hikerID.isValid()) {
		int bufferRowIndex = db->hikersTable->getBufferIndexForPrimaryKey(hikerID.get());
		defaultHikerCombo->setCurrentIndex(bufferRowIndex + 1);	// 0 is None
	} else {
		defaultHikerCombo->setCurrentIndex(0);
	}
	
	usePhotosBasePathCheckbox->setChecked(db->projectSettings->getValue(db->projectSettings->usePhotosBasePath).toBool());
	photosBasePathLineEdit->setText(db->projectSettings->getValue(db->projectSettings->photosBasePath).toString());
	
	updateEnabled();
}

void ProjectSettingsWindow::saveSettings()
{
	db->projectSettings->setValue(this, db->projectSettings->defaultHiker,		parseIDCombo(defaultHikerCombo).asQVariant());
	db->projectSettings->setValue(this, db->projectSettings->usePhotosBasePath,	parseCheckbox(usePhotosBasePathCheckbox));
	db->projectSettings->setValue(this, db->projectSettings->photosBasePath,	parseLineEdit(photosBasePathLineEdit));
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