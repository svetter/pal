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

#include "settings.h"



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

QString getDefaultLanguage()
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





QString Settings::getAppVersion()
{
	return QString("%1.%2.%3")
		.arg(APP_VERSION_MAJOR)
		.arg(APP_VERSION_MINOR)
		.arg(APP_VERSION_PATCH);
}

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

bool Settings::settingsOlderThan(QString version)
{
	return isBelowVersion(appVersion.get(), version);
}



void Settings::checkForVersionChange()
{
	// 1.1.0: New columns => reset column widths
	if (settingsOlderThan("1.1.0")) {
		resetColumnWidths();
	}
	
	// Update settings version
	QString currentAppVersion = getAppVersion();
	if (settingsOlderThan(currentAppVersion)) {
		qDebug().noquote().nospace() << "Updating settings from v" << appVersion.get() << " to v" << currentAppVersion;
		appVersion.set(currentAppVersion);
	}
}





void saveDialogGeometry(QWidget* dialog, QWidget* parent, const Setting<QRect>* geometrySetting)
{
	QRect absoluteGeometry = dialog->geometry();
	if (Settings::rememberWindowPositionsRelative.get()) {
		absoluteGeometry.adjust(- parent->x(), - parent->y(), 0, 0);
	}
	geometrySetting->set(absoluteGeometry);
}

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
