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

#include "main_window.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>



void configureTranslation(QApplication& application)
{
	QString languageSetting = Settings::language.get();
	bool useTranslator = languageSetting != "en";
	
	if (useTranslator) {
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
}



int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	
	if (argc >= 2) {
		application.setStyle(argv[1]);
	}
	
	// Update settings if necessary
	Settings::checkForVersionChange();
	
	configureTranslation(application);
	
	MainWindow mainWindow = MainWindow();
	mainWindow.show();
	return application.exec();
}
