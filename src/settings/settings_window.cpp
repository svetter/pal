/*
 * Copyright 2023-2024 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file settings_window.cpp
 * 
 * This file defines the SettingsWindow class.
 */

#include "settings_window.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>



/**
 * Creates a new SettingsWindow.
 * 
 * Restores geometry if the setting is enabled, handles signal connections, populates the language
 * combo box and loads the settings into the UI.
 * 
 * @param mainWindow	The application's main window.
 */
SettingsWindow::SettingsWindow(QMainWindow& mainWindow) :
	QDialog(&mainWindow),
	mainWindow(mainWindow),
	languages(getSupportedLanguages()),
	styles(getSupportedStyles()),
	liveStyleUpdates(false)
{
	setupUi(this);
	
	restoreDialogGeometry(*this, mainWindow, Settings::settingsWindow_geometry);
	
	
	connect(styleCombo,							&QComboBox::currentIndexChanged,	this,	&SettingsWindow::applySelectedStyle);
	connect(rememberWindowGeometryCheckbox,		&QCheckBox::stateChanged,			this,	&SettingsWindow::handle_rememberWindowPositionsCheckboxChanged);
	connect(ascentDateCheckbox,					&QCheckBox::stateChanged,			this,	&SettingsWindow::handle_ascentDateCheckboxChanged);
	connect(ascentTimeCheckbox,					&QCheckBox::stateChanged,			this,	&SettingsWindow::handle_ascentTimeCheckboxChanged);
	connect(ascentElevationGainCheckbox,		&QCheckBox::stateChanged,			this,	&SettingsWindow::handle_ascentElevationGainCheckboxChanged);
	connect(peakHeightCheckbox,					&QCheckBox::stateChanged,			this,	&SettingsWindow::handle_peakHeightCheckboxChanged);
	
	connect(bottomButtonBox->button(QDialogButtonBox::Save),			&QPushButton::clicked,	this,	&SettingsWindow::handle_save);
	connect(bottomButtonBox->button(QDialogButtonBox::Apply),			&QPushButton::clicked,	this,	&SettingsWindow::handle_apply);
	connect(bottomButtonBox->button(QDialogButtonBox::Cancel),			&QPushButton::clicked,	this,	&SettingsWindow::handle_cancel);
	connect(bottomButtonBox->button(QDialogButtonBox::RestoreDefaults),	&QPushButton::clicked,	this,	&SettingsWindow::handle_loadDefaults);
	
	
	languageCombo	->addItems(languages.second);
	styleCombo		->addItems(styles.second);
	
	
	loadSettings();
	
	// Live style updates disabled because a Qt bug resets table column widths when setting style
	//liveStyleUpdates = true;
}



/**
 * Loads all represented settings into their respective UI elements.
 * 
 * For items which are not present in the settings, the default value is loaded.
 */
void SettingsWindow::loadSettings()
{
	int languageIndex = languages.first.indexOf(language.get());
	if (languageIndex < 0) {
		qDebug() << "Couldn't parse language setting, reverting to default";
		languageIndex = languages.first.indexOf(language.getDefault());
	}
	languageCombo->setCurrentIndex(languageIndex);
	
	int styleIndex = styles.first.indexOf(uiStyle.get());
	if (styleIndex < 0) {
		qDebug() << "Couldn't parse style setting, reverting to default";
		styleIndex = styles.first.indexOf(language.getDefault());
		if (styleIndex < 0) styleIndex = 0;
	}
	styleCombo->setCurrentIndex(styleIndex);
	
	confirmDeleteCheckbox						->setChecked	(confirmDelete								.get());
	confirmCancelCheckbox						->setChecked	(confirmCancel								.get());
	warnAboutDuplicateNamesCheckbox				->setChecked	(warnAboutDuplicateNames					.get());
	onlyPrepareActiveTableCheckbox				->setChecked	(onlyPrepareActiveTableOnStartup			.get());
	openProjectSettingsOnNewDatabaseCheckbox	->setChecked	(openProjectSettingsOnNewDatabase			.get());
	rememberWindowGeometryCheckbox				->setChecked	(rememberWindowPositions					.get());
	rememberWindowPositionsRelativeCheckbox		->setChecked	(rememberWindowPositionsRelative			.get());
	rememberTableCheckbox						->setChecked	(rememberTab								.get());
	rememberColumnWidthsCheckbox				->setChecked	(rememberColumnWidths						.get());
	rememberColumnOrderCheckbox					->setChecked	(rememberColumnOrder						.get());
	rememberHiddenColumnsCheckbox				->setChecked	(rememberHiddenColumns						.get());
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
	
	googleApiKeyEdit							->setText		(googleApiKey								.get());
	
	ascentTitleUnderPeakNameCheckbox			->setChecked	(ascentViewer_ascentTitleUnderPeakName		.get());
	
	updateEnabled();
}

/**
 * Loads the default values for all settings into their respective UI elements.
 */
void SettingsWindow::loadDefaults()
{
	int languageIndex = languages.first.indexOf(language.getDefault());
	languageCombo->setCurrentIndex(languageIndex);
	int styleIndex = styles.first.indexOf(uiStyle.getDefault());
	styleCombo->setCurrentIndex(styleIndex);
	
	confirmDeleteCheckbox						->setChecked	(confirmDelete								.getDefault());
	confirmCancelCheckbox						->setChecked	(confirmCancel								.getDefault());
	warnAboutDuplicateNamesCheckbox				->setChecked	(warnAboutDuplicateNames					.getDefault());
	onlyPrepareActiveTableCheckbox				->setChecked	(onlyPrepareActiveTableOnStartup			.getDefault());
	openProjectSettingsOnNewDatabaseCheckbox	->setChecked	(openProjectSettingsOnNewDatabase			.getDefault());
	rememberWindowGeometryCheckbox				->setChecked	(rememberWindowPositions					.getDefault());
	rememberWindowPositionsRelativeCheckbox		->setChecked	(rememberWindowPositionsRelative			.getDefault());
	rememberTableCheckbox						->setChecked	(rememberTab								.getDefault());
	rememberColumnWidthsCheckbox				->setChecked	(rememberColumnWidths						.getDefault());
	rememberColumnOrderCheckbox					->setChecked	(rememberColumnOrder						.getDefault());
	rememberHiddenColumnsCheckbox				->setChecked	(rememberHiddenColumns						.getDefault());
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
	
	googleApiKeyEdit							->setText		(googleApiKey								.getDefault());
	
	ascentTitleUnderPeakNameCheckbox			->setChecked	(ascentViewer_ascentTitleUnderPeakName		.getDefault());
	
	updateEnabled();
}

/**
 * Saves all represented settings from their respective UI elements.
 */
void SettingsWindow::saveSettings()
{
	QString languageBefore = language.get();
	bool rememberWindowPositionsRelativeBefore = rememberWindowPositionsRelative.get();
	
	int selectedLanguageIndex = languageCombo->currentIndex();
	if (selectedLanguageIndex >= 0) {
		language.set(languages.first.at(selectedLanguageIndex));
	}
	
	int selectedStyleIndex = styleCombo->currentIndex();
	if (selectedStyleIndex >= 0) {
		uiStyle.set(styles.first.at(selectedStyleIndex));
	}
	
	confirmDelete								.set(confirmDeleteCheckbox						->isChecked());
	confirmCancel								.set(confirmCancelCheckbox						->isChecked());
	warnAboutDuplicateNames						.set(warnAboutDuplicateNamesCheckbox			->isChecked());
	onlyPrepareActiveTableOnStartup				.set(onlyPrepareActiveTableCheckbox				->isChecked());
	openProjectSettingsOnNewDatabase			.set(openProjectSettingsOnNewDatabaseCheckbox	->isChecked());
	rememberWindowPositions						.set(rememberWindowGeometryCheckbox				->isChecked());
	rememberWindowPositionsRelative				.set(rememberWindowPositionsRelativeCheckbox	->isChecked());
	rememberTab									.set(rememberTableCheckbox						->isChecked());
	rememberColumnWidths						.set(rememberColumnWidthsCheckbox				->isChecked());
	rememberColumnOrder							.set(rememberColumnOrderCheckbox				->isChecked());
	rememberHiddenColumns						.set(rememberHiddenColumnsCheckbox				->isChecked());
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
	
	googleApiKey								.set(googleApiKeyEdit							->text());
	
	ascentViewer_ascentTitleUnderPeakName		.set(ascentTitleUnderPeakNameCheckbox			->isChecked());
	
	if (languageBefore != language.get()) {
		QString title = tr("Language setting changed");
		QString message = tr("Changing the language requires a restart.");
		QMessageBox::information(this, title, message);
	}
	
	if (rememberWindowPositionsRelativeBefore != rememberWindowPositionsRelative.get()) {
		Settings::resetGeometrySettings();
	}
}

/**
 * Updates the enabled state of all UI elements which depend on the state of other UI elements.
 */
void SettingsWindow::updateEnabled()
{
	handle_rememberWindowPositionsCheckboxChanged();
	handle_ascentDateCheckboxChanged();
	handle_ascentTimeCheckboxChanged();
	handle_ascentElevationGainCheckboxChanged();
	handle_peakHeightCheckboxChanged();
}



/**
 * Event handler for rememberWindowPositionsCheckbox.
 * 
 * Enables or disables rememberWindowPositionsRelativeCheckbox.
 */
void SettingsWindow::handle_rememberWindowPositionsCheckboxChanged()
{
	bool enabled = rememberWindowGeometryCheckbox->checkState();
	rememberWindowPositionsRelativeCheckbox->setEnabled(enabled);
}


/**
 * Event handler for ascentDateCheckbox.
 * 
 * Enables or disables ascentDateDaysInPastSpinner and ascentDateDaysInPastLabel.
 */
void SettingsWindow::handle_ascentDateCheckboxChanged()
{
	bool enabled = ascentDateCheckbox->checkState();
	ascentDateDaysInPastSpinner->setEnabled(enabled);
	ascentDateDaysInPastLabel->setEnabled(enabled);
}

/**
 * Event handler for ascentTimeCheckbox.
 * 
 * Enables or disables ascentTimeWidget.
 */
void SettingsWindow::handle_ascentTimeCheckboxChanged()
{
	bool enabled = ascentTimeCheckbox->checkState();
	ascentTimeWidget->setEnabled(enabled);
}

/**
 * Event handler for ascentElevationGainCheckbox.
 * 
 * Enables or disables ascentElevationGainSpinner.
 */
void SettingsWindow::handle_ascentElevationGainCheckboxChanged()
{
	bool enabled = ascentElevationGainCheckbox->checkState();
	ascentElevationGainSpinner->setEnabled(enabled);
}

/**
 * Event handler for peakHeightCheckbox.
 * 
 * Enables or disables peakHeightSpinner.
 */
void SettingsWindow::handle_peakHeightCheckboxChanged()
{
	bool enabled = peakHeightCheckbox->checkState();
	peakHeightSpinner->setEnabled(enabled);
}



/**
 * Event handler for the save button.
 * 
 * Saves the settings, window position and size and closes the window.
 */
void SettingsWindow::handle_save()
{
	saveSettings();
	saveDialogGeometry(*this, mainWindow, Settings::settingsWindow_geometry);
	accept();
}

/**
 * Event handler for the apply button.
 * 
 * Saves the settings.
 */
void SettingsWindow::handle_apply()
{
	saveSettings();
}

/**
 * Event handler for the cancel button.
 * 
 * Saves the window position and size and closes the window.
 */
void SettingsWindow::handle_cancel()
{
	saveDialogGeometry(*this, mainWindow, Settings::settingsWindow_geometry);
	setVisible(false);
	applyStoredStyle();
	QDialog::reject();
}

/**
 * Event handler for the restore defaults button.
 * 
 * Loads the default settings.
 */
void SettingsWindow::handle_loadDefaults()
{
	loadDefaults();
	applySelectedStyle();
}



/**
 * Parses the currently selected style from the style combo box and applies it if live style updates
 * are enabled.
 */
void SettingsWindow::applySelectedStyle()
{
	if (!liveStyleUpdates) return;

	int selectedStyleIndex = styleCombo->currentIndex();
	if (selectedStyleIndex < 0) return;
	QString selectedStyle = styles.first.at(selectedStyleIndex);
	applyStyle(selectedStyle);
}

/**
 * Fetchesthe style saved in the settings and applies it if live style updates are enabled.
 */
void SettingsWindow::applyStoredStyle()
{
	if (!liveStyleUpdates) return;

	applyStyle(uiStyle.get());
}

/**
 * Applies the given style to the application.
 * 
 * @param styleString	The style to apply.
 */
void SettingsWindow::applyStyle(QString styleString)
{
	if (styleString.isEmpty()) {
		styleString = Settings::systemDefaultStyle;
	}
	QApplication* application = qobject_cast<QApplication*>(QCoreApplication::instance());
	application->setStyle(styleString);
}



/**
 * Event handler for the close event.
 * 
 * Forwarded to the cancel button event handler.
 */
void SettingsWindow::reject()
{
	handle_cancel();
}
