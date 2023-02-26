#include "settings_window.h"

#include <QDialogButtonBox>
#include <QPushButton>



SettingsWindow::SettingsWindow(QWidget* parent) :
		QDialog(parent)
{
	setupUi(this);
	
	const QRect savedGeometry = Settings::settingsWindow_geometry.get();
	if (!savedGeometry.isEmpty()) {
		setGeometry(savedGeometry);
	}
	setFixedSize(minimumSizeHint());
	
	
	connect(ascentDateCheckbox,				&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_ascentDateCheckboxChanged);
	connect(ascentTimeCheckbox,				&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_ascentTimeCheckboxChanged);
	connect(ascentElevationGainCheckbox,	&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_ascentElevationGainCheckboxChanged);
	connect(peakHeightCheckbox,				&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_peakHeightCheckboxChanged);
	
	connect(bottomButtonBox->button(QDialogButtonBox::Save),			&QPushButton::clicked,	this,	&SettingsWindow::handle_save);
	connect(bottomButtonBox->button(QDialogButtonBox::Apply),			&QPushButton::clicked,	this,	&SettingsWindow::handle_apply);
	connect(bottomButtonBox->button(QDialogButtonBox::Cancel),			&QPushButton::clicked,	this,	&SettingsWindow::handle_cancel);
	connect(bottomButtonBox->button(QDialogButtonBox::RestoreDefaults),	&QPushButton::clicked,	this,	&SettingsWindow::handle_loadDefaults);
	
	
	loadSettings();
}



void SettingsWindow::loadSettings()
{
	confirmDeleteCheckbox			->setChecked	(confirmDelete								.get());
	confirmCancelCheckbox			->setChecked	(confirmCancel								.get());
	allowEmptyNamesCheckbox			->setChecked	(allowEmptyNames							.get());
	rememberWindowPositionsCheckbox	->setChecked	(rememberWindowPositions					.get());
	rememberTabCheckbox				->setChecked	(mainWindow_rememberTab						.get());
	rememberColumnWidthsCheckbox	->setChecked	(mainWindow_rememberColumnWidths			.get());
	
	ascentDateCheckbox				->setChecked	(ascentDialog_dateEnabledInitially			.get());
	ascentDateDaysInPastSpinner		->setValue		(ascentDialog_initialDateDaysInPast			.get());
	ascentTimeCheckbox				->setChecked	(ascentDialog_timeEnabledInitially			.get());
	ascentTimeWidget				->setTime		(ascentDialog_initialTime					.get());
	ascentElevationGainCheckbox		->setChecked	(ascentDialog_elevationGainEnabledInitially	.get());
	ascentElevationGainSpinner		->setValue		(ascentDialog_initialElevationGain			.get());
	
	peakHeightCheckbox				->setChecked	(peakDialog_heightEnabledInitially			.get());
	peakHeightSpinner				->setValue		(peakDialog_initialHeight					.get());
	
	tripDatesCheckbox				->setChecked	(tripDialog_datesEnabledInitially			.get());
	
	updateEnabled();
}

void SettingsWindow::loadDefaults()
{
	confirmDeleteCheckbox			->setChecked	(confirmDelete								.getDefault());
	confirmCancelCheckbox			->setChecked	(confirmCancel								.getDefault());
	allowEmptyNamesCheckbox			->setChecked	(allowEmptyNames							.getDefault());
	rememberWindowPositionsCheckbox	->setChecked	(rememberWindowPositions					.getDefault());
	rememberTabCheckbox				->setChecked	(mainWindow_rememberTab						.getDefault());
	rememberColumnWidthsCheckbox	->setChecked	(mainWindow_rememberColumnWidths			.getDefault());
	
	ascentDateCheckbox				->setChecked	(ascentDialog_dateEnabledInitially			.getDefault());
	ascentDateDaysInPastSpinner		->setValue		(ascentDialog_initialDateDaysInPast			.getDefault());
	ascentTimeCheckbox				->setChecked	(ascentDialog_timeEnabledInitially			.getDefault());
	ascentTimeWidget				->setTime		(ascentDialog_initialTime					.getDefault());
	ascentElevationGainCheckbox		->setChecked	(ascentDialog_elevationGainEnabledInitially	.getDefault());
	ascentElevationGainSpinner		->setValue		(ascentDialog_initialElevationGain			.getDefault());
	
	peakHeightCheckbox				->setChecked	(peakDialog_heightEnabledInitially			.getDefault());
	peakHeightSpinner				->setValue		(peakDialog_initialHeight					.getDefault());
	
	tripDatesCheckbox				->setChecked	(tripDialog_datesEnabledInitially			.getDefault());
	
	updateEnabled();
}

void SettingsWindow::saveSettings()
{
	confirmDelete								.set(confirmDeleteCheckbox				->isChecked());
	confirmCancel								.set(confirmCancelCheckbox				->isChecked());
	allowEmptyNames								.set(allowEmptyNamesCheckbox			->isChecked());
	rememberWindowPositions						.set(rememberWindowPositionsCheckbox	->isChecked());
	mainWindow_rememberTab						.set(rememberTabCheckbox				->isChecked());
	mainWindow_rememberColumnWidths				.set(rememberColumnWidthsCheckbox		->isChecked());
	
	ascentDialog_dateEnabledInitially			.set(ascentDateCheckbox					->isChecked());
	ascentDialog_initialDateDaysInPast			.set(ascentDateDaysInPastSpinner		->value());
	ascentDialog_timeEnabledInitially			.set(ascentTimeCheckbox					->isChecked());
	ascentDialog_initialTime					.set(ascentTimeWidget					->time());
	ascentDialog_elevationGainEnabledInitially	.set(ascentElevationGainCheckbox		->isChecked());
	ascentDialog_initialElevationGain			.set(ascentElevationGainSpinner			->value());
	
	peakDialog_heightEnabledInitially			.set(peakHeightCheckbox					->isChecked());
	peakDialog_initialHeight					.set(peakHeightSpinner					->value());
	
	tripDialog_datesEnabledInitially			.set(tripDatesCheckbox					->isChecked());
}

void SettingsWindow::updateEnabled()
{
	handle_ascentDateCheckboxChanged();
	handle_ascentTimeCheckboxChanged();
	handle_ascentElevationGainCheckboxChanged();
	handle_peakHeightCheckboxChanged();
}



void SettingsWindow::handle_ascentDateCheckboxChanged()
{
	bool enabled = ascentDateCheckbox->checkState();
	ascentDateDaysInPastSpinner->setEnabled(enabled);
	ascentDateDaysInPastLabel->setEnabled(enabled);
}

void SettingsWindow::handle_ascentTimeCheckboxChanged()
{
	bool enabled = ascentTimeCheckbox->checkState();
	ascentTimeWidget->setEnabled(enabled);
}

void SettingsWindow::handle_ascentElevationGainCheckboxChanged()
{
	bool enabled = ascentElevationGainCheckbox->checkState();
	ascentElevationGainSpinner->setEnabled(enabled);
}

void SettingsWindow::handle_peakHeightCheckboxChanged()
{
	bool enabled = peakHeightCheckbox->checkState();
	peakHeightSpinner->setEnabled(enabled);
}



void SettingsWindow::handle_save()
{
	Settings::settingsWindow_geometry.set(geometry());
	
	saveSettings();
}

void SettingsWindow::handle_apply()
{
	saveSettings();
}

void SettingsWindow::handle_cancel()
{
	Settings::settingsWindow_geometry.set(geometry());
}

void SettingsWindow::handle_loadDefaults()
{
	loadDefaults();
}



void SettingsWindow::reject()
{
	handle_cancel();
	QDialog::reject();
}