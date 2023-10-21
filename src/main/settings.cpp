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
