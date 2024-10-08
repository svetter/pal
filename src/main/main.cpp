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
 * @file main.cpp
 * 
 * Main entry point of the application and top level configuration.
 * 
 * This file contains the main entry point of the application as well as configuration
 * for Qt style, translation and setting version checking.
 */

#include "main_window.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>



/**
 * Configures the translation of the application.
 * 
 * @param application	Initialized QApplication to configure the translation for.
 */
void configureTranslation(QApplication& application)
{
	const QString languageSetting = Settings::language.get();
	
	if (languageSetting != "en") {
		// Base translator: Translating strings from Qt framework
		QTranslator* baseTranslator = new QTranslator();
		if (baseTranslator->load("qtbase_" + QLocale(languageSetting).name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
			if (!application.installTranslator(baseTranslator)) {
				qDebug() << "Installing base translator for" << languageSetting << "failed";
			} else {
				qDebug() << "Installed base translator from" << baseTranslator->filePath();
			}
		} else {
			qDebug() << "Base translator for configured language" << languageSetting << "not found";
		}
	}
	
	// App translator: Translating this application's own strings
	QTranslator* appTranslator = new QTranslator();
	if (appTranslator->load(QLocale(languageSetting).name(), ":/i18n/")) {
		if (appTranslator->isEmpty()) {
			qDebug() << "Translator" << appTranslator->filePath() << "is empty";
		}
		
		if (!application.installTranslator(appTranslator)) {
			qDebug() << "Installing translator for" << languageSetting << "failed";
		} else {
			qDebug() << "Installed translator from" << appTranslator->filePath();
		}
	} else {
		qDebug() << "Translator for configured language" << languageSetting << "not found";
	}
}



/**
 * Main entry point of the application.
 * 
 * @param argc	Number of command line arguments.
 * @param argv	Command line arguments.
 * @return		Exit code.
 */
int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	
	Settings::systemDefaultStyle = application.style()->objectName();
	
	// Update settings if necessary
	Settings::checkForVersionChange();
	
	const QString styleSetting = Settings::uiStyle.get();
	if (!styleSetting.isEmpty()) {
		applyStyle(styleSetting);
	}
	const QString colorSchemeSetting = Settings::uiColorScheme.get();
	if (!colorSchemeSetting.isEmpty()) {
		applyColorScheme(colorSchemeSetting);
	}
	
	configureTranslation(application);
	
	MainWindow mainWindow = MainWindow();
	mainWindow.show();
	return application.exec();
}
