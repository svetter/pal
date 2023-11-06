/*
 * Copyright 2023 Simon Vetter
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
 * @file settings.cpp
 * 
 * This file defines part of the Settings class.
 */

#include "settings.h"



/**
 * Lists all languages supported by the application and returns a two lists of the language codes
 * and the native language names, respectively.
 * 
 * @return A pair of two lists, the first containing the language codes and the second containing the native language names.
 */
QPair<QStringList, QStringList> getSupportedLanguages()
{
	QStringList codes = QStringList("en");
	QStringList names = QStringList("English");
	
	QDirIterator it = QDirIterator(":/i18n/", QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString code = it.next().split(".").at(0).split("/").at(2);
		codes.append(code);
		names.append(QLocale(code).nativeLanguageName());
	}
	
	return {codes, names};
}

/**
 * Determines which language should be used by default for the application, based on the user's
 * system language and the languages supported by the application.
 * 
 * @return The language code of the language that should be used by default on the current system.
 */
QString getDefaultLanguageCode()
{
	QString language = "en";
	QStringList uiLanguages = QLocale::system().uiLanguages();
	QStringList supportedLanguages = getSupportedLanguages().first;
	
	for (const QString& locale : uiLanguages) {
		const QString preferredLanguage = QLocale(locale).name();
		if (supportedLanguages.contains(preferredLanguage)) {
			language = preferredLanguage;
			break;
		}
	}
	return language;
}





/**
 * Returns a string containing the application's version number.
 * 
 * @return The application's version number in string form.
 */
QString Settings::getAppVersion()
{
	return QString("%1.%2.%3")
		.arg(APP_VERSION_MAJOR)
		.arg(APP_VERSION_MINOR)
		.arg(APP_VERSION_PATCH);
}

/**
 * Compares two version strings and determines whether the first one is older than the second one.
 * 
 * @param settingsVersion The version of the application that saved the settings file.
 * @param minimalVersion The version of the application with which to compare the settings file version.
 * @return True if the settings file is older than the given version, false otherwise.
 */
bool Settings::isBelowVersion(QString settingsVersion, QString minimalVersion)
{
	QStringList settingsSplit	= settingsVersion.split('.');
	QStringList minimalSplit	= minimalVersion.split('.');
	assert(settingsSplit.size() == 3 && minimalSplit.size() == 3);
	for (int i = 0; i < 3; i++) {
		bool conversionOk = false;
		int settingsNumber = settingsSplit.at(i).toInt(&conversionOk);
		assert(conversionOk);
		int minimalNumber = minimalSplit.at(i).toInt(&conversionOk);
		assert(conversionOk);
		if (settingsNumber < minimalNumber) return true;
	}
	return false;
}

/**
 * Determines whether the settings file was last saved by a version of the application that is
 * older than the current version.
 * 
 * @param version The version of the application with which to compare the settings file version.
 * @return True if the settings file is older than the current version, false otherwise.
 */
bool Settings::settingsOlderThan(QString version)
{
	return isBelowVersion(appVersion.get(), version);
}



/**
 * Updates the settings file to the current version of the application.
 */
void Settings::checkForVersionChange()
{
	// 1.1.0: New columns => reset column widths
	if (settingsOlderThan("1.1.0")) {
		resetColumnWidths();
	}
	
	// 1.1.2: New splitter in ascent viewer => remove deprecated setting
	if (settingsOlderThan("1.1.2")) {
		qSettings.remove("implicit/ascentViewer/splitterSizes");
	}
	
	// Update settings version
	QString currentAppVersion = getAppVersion();
	if (settingsOlderThan(currentAppVersion)) {
		qDebug().noquote().nospace() << "Updating settings from v" << appVersion.get() << " to v" << currentAppVersion;
		appVersion.set(currentAppVersion);
	}
}





/**
 * Stores implicit settings about position and geometry for the given dialog.
 * 
 * @param dialog			The dialog for which to store the geometry.
 * @param parent			The parent widget of the dialog (for determining relative position).
 * @param geometrySetting	The geometry setting corresponding to the dialog.
 */
void saveDialogGeometry(QWidget* dialog, QWidget* parent, const Setting<QRect>* geometrySetting)
{
	QRect absoluteGeometry = dialog->geometry();
	if (Settings::rememberWindowPositionsRelative.get()) {
		absoluteGeometry.adjust(- parent->x(), - parent->y(), 0, 0);
	}
	geometrySetting->set(absoluteGeometry);
}

/**
 * Restores implicit settings about position and geometry for the given dialog.
 * 
 * @param dialog			The dialog for which to restore the geometry.
 * @param parent			The parent widget of the dialog (for restoring relative position).
 * @param geometrySetting	The geometry setting corresponding to the dialog.
 */
void restoreDialogGeometry(QWidget* dialog, QWidget* parent, const Setting<QRect>* geometrySetting)
{
	if (!Settings::rememberWindowPositions.present()) return;
	
	QRect savedGeometry = geometrySetting->get();
	if (savedGeometry.isEmpty()) return;
	
	if (Settings::rememberWindowPositionsRelative.get()) {
		savedGeometry.adjust(parent->x(), parent->y(), 0, 0);
	}
	
	dialog->setGeometry(savedGeometry);
}
