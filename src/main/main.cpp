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



int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	
	
	
	// Configure translation
	
	QString languageSetting = Settings::language.get();
	bool useTranslator = languageSetting != "en";
	QTranslator translator;
	
	if (useTranslator) {
		if (translator.load(QLocale(languageSetting).name(), ":/i18n/")) {
			if (translator.isEmpty()) {
				qDebug() << "Translator" << translator.filePath() << "is empty";
			} else {
				qDebug() << "Loaded translator from" << translator.filePath();
			}
			
			if (!application.installTranslator(&translator)) {
				qDebug() << "Installing translator for" << languageSetting << "failed";
			}
		} else {
			qDebug() << "Translator for configured language" << languageSetting << "not found";
		}
	}
	
	
	
	MainWindow mainWindow = MainWindow();
	
	mainWindow.show();
	return application.exec();
}
