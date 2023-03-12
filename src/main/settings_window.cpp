#include "settings_window.h"

#include <QDialogButtonBox>
#include <QPushButton>



SettingsWindow::SettingsWindow(QWidget* parent) :
		QDialog(parent),
		parent(parent)
{
	setupUi(this);
	
	restoreDialogGeometry(this, parent, &Settings::settingsWindow_geometry);
	
	
	connect(rememberWindowPositionsCheckbox,	&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_rememberWindowPositionsCheckboxChanged);
	connect(ascentDateCheckbox,					&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_ascentDateCheckboxChanged);
	connect(ascentTimeCheckbox,					&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_ascentTimeCheckboxChanged);
	connect(ascentElevationGainCheckbox,		&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_ascentElevationGainCheckboxChanged);
	connect(peakHeightCheckbox,					&QCheckBox::stateChanged,	this,	&SettingsWindow::handle_peakHeightCheckboxChanged);
	
	connect(bottomButtonBox->button(QDialogButtonBox::Save),			&QPushButton::clicked,	this,	&SettingsWindow::handle_save);
	connect(bottomButtonBox->button(QDialogButtonBox::Apply),			&QPushButton::clicked,	this,	&SettingsWindow::handle_apply);
	connect(bottomButtonBox->button(QDialogButtonBox::Cancel),			&QPushButton::clicked,	this,	&SettingsWindow::handle_cancel);
	connect(bottomButtonBox->button(QDialogButtonBox::RestoreDefaults),	&QPushButton::clicked,	this,	&SettingsWindow::handle_loadDefaults);
	
	
	loadSettings();
}



void SettingsWindow::loadSettings()
{
	confirmDeleteCheckbox						->setChecked	(confirmDelete								.get());
	confirmCancelCheckbox						->setChecked	(confirmCancel								.get());
	allowEmptyNamesCheckbox						->setChecked	(allowEmptyNames							.get());
	openProjectSettingsOnNewDatabaseCheckbox	->setChecked	(openProjectSettingsOnNewDatabase			.get());
	rememberWindowPositionsCheckbox				->setChecked	(rememberWindowPositions					.get());
	rememberWindowPositionsRelativeCheckbox		->setChecked	(rememberWindowPositionsRelative			.get());
	rememberTableCheckbox						->setChecked	(rememberTab								.get());
	rememberColumnWidthsCheckbox				->setChecked	(rememberColumnWidths						.get());
	rememberSortingCheckbox						->setChecked	(rememberSorting							.get());
	rememberFiltersCheckbox						->setChecked	(rememberFilters							.get());
	
	ascentDateCheckbox							->setChecked	(ascentDialog_dateEnabledInitially			.get());
	ascentDateDaysInPastSpinner					->setValue		(ascentDialog_initialDateDaysInPast			.get());
	ascentTimeCheckbox							->setChecked	(ascentDialog_timeEnabledInitially			.get());
	ascentTimeWidget							->setTime		(ascentDialog_initialTime					.get());
	ascentElevationGainCheckbox					->setChecked	(ascentDialog_elevationGainEnabledInitially	.get());
	ascentElevationGainSpinner					->setValue		(ascentDialog_initialElevationGain			.get());
	
	peakHeightCheckbox							->setChecked	(peakDialog_heightEnabledInitially			.get());
	peakHeightSpinner							->setValue		(peakDialog_initialHeight					.get());
	
	tripDatesCheckbox							->setChecked	(tripDialog_datesEnabledInitially			.get());
	
	updateEnabled();
}

void SettingsWindow::loadDefaults()
{
	confirmDeleteCheckbox						->setChecked	(confirmDelete								.getDefault());
	confirmCancelCheckbox						->setChecked	(confirmCancel								.getDefault());
	allowEmptyNamesCheckbox						->setChecked	(allowEmptyNames							.getDefault());
	openProjectSettingsOnNewDatabaseCheckbox	->setChecked	(openProjectSettingsOnNewDatabase			.getDefault());
	rememberWindowPositionsCheckbox				->setChecked	(rememberWindowPositions					.getDefault());
	rememberWindowPositionsRelativeCheckbox		->setChecked	(rememberWindowPositionsRelative			.getDefault());
	rememberTableCheckbox						->setChecked	(rememberTab								.getDefault());
	rememberColumnWidthsCheckbox				->setChecked	(rememberColumnWidths						.getDefault());
	rememberSortingCheckbox						->setChecked	(rememberSorting							.getDefault());
	rememberFiltersCheckbox						->setChecked	(rememberFilters							.getDefault());
	
	ascentDateCheckbox							->setChecked	(ascentDialog_dateEnabledInitially			.getDefault());
	ascentDateDaysInPastSpinner					->setValue		(ascentDialog_initialDateDaysInPast			.getDefault());
	ascentTimeCheckbox							->setChecked	(ascentDialog_timeEnabledInitially			.getDefault());
	ascentTimeWidget							->setTime		(ascentDialog_initialTime					.getDefault());
	ascentElevationGainCheckbox					->setChecked	(ascentDialog_elevationGainEnabledInitially	.getDefault());
	ascentElevationGainSpinner					->setValue		(ascentDialog_initialElevationGain			.getDefault());
	
	peakHeightCheckbox							->setChecked	(peakDialog_heightEnabledInitially			.getDefault());
	peakHeightSpinner							->setValue		(peakDialog_initialHeight					.getDefault());
	
	tripDatesCheckbox							->setChecked	(tripDialog_datesEnabledInitially			.getDefault());
	
	updateEnabled();
}

void SettingsWindow::saveSettings()
{
	bool rememberWindowPositionsRelativeBefore = rememberWindowPositionsRelative.get();
	
	confirmDelete								.set(confirmDeleteCheckbox						->isChecked());
	confirmCancel								.set(confirmCancelCheckbox						->isChecked());
	allowEmptyNames								.set(allowEmptyNamesCheckbox					->isChecked());
	openProjectSettingsOnNewDatabase			.set(openProjectSettingsOnNewDatabaseCheckbox	->isChecked());
	rememberWindowPositions						.set(rememberWindowPositionsCheckbox			->isChecked());
	rememberWindowPositionsRelative				.set(rememberWindowPositionsRelativeCheckbox	->isChecked());
	rememberTab									.set(rememberTableCheckbox						->isChecked());
	rememberColumnWidths						.set(rememberColumnWidthsCheckbox				->isChecked());
	rememberSorting								.set(rememberSortingCheckbox					->isChecked());
	rememberFilters								.set(rememberFiltersCheckbox					->isChecked());
	
	ascentDialog_dateEnabledInitially			.set(ascentDateCheckbox							->isChecked());
	ascentDialog_initialDateDaysInPast			.set(ascentDateDaysInPastSpinner				->value());
	ascentDialog_timeEnabledInitially			.set(ascentTimeCheckbox							->isChecked());
	ascentDialog_initialTime					.set(ascentTimeWidget							->time());
	ascentDialog_elevationGainEnabledInitially	.set(ascentElevationGainCheckbox				->isChecked());
	ascentDialog_initialElevationGain			.set(ascentElevationGainSpinner					->value());
	
	peakDialog_heightEnabledInitially			.set(peakHeightCheckbox							->isChecked());
	peakDialog_initialHeight					.set(peakHeightSpinner							->value());
	
	tripDialog_datesEnabledInitially			.set(tripDatesCheckbox							->isChecked());
	
	if (rememberWindowPositionsRelativeBefore != rememberWindowPositionsRelative.get()) {
		Settings::resetGeometrySettings();
	}
}

void SettingsWindow::updateEnabled()
{
	handle_rememberWindowPositionsCheckboxChanged();
	handle_ascentDateCheckboxChanged();
	handle_ascentTimeCheckboxChanged();
	handle_ascentElevationGainCheckboxChanged();
	handle_peakHeightCheckboxChanged();
}



void SettingsWindow::handle_rememberWindowPositionsCheckboxChanged()
{
	bool enabled = rememberWindowPositionsCheckbox->checkState();
	rememberWindowPositionsRelativeCheckbox->setEnabled(enabled);
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
	saveSettings();
	saveDialogGeometry(this, parent, &Settings::settingsWindow_geometry);
	accept();
}

void SettingsWindow::handle_apply()
{
	saveSettings();
}

void SettingsWindow::handle_cancel()
{
	saveDialogGeometry(this, parent, &Settings::settingsWindow_geometry);
	QDialog::reject();
}

void SettingsWindow::handle_loadDefaults()
{
	loadDefaults();
}



void SettingsWindow::reject()
{
	handle_cancel();
}