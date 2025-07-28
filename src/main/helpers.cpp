/*
 * Copyright 2023-2025 Simon Vetter
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
 * @file helpers.cpp
 * 
 * This file defines general static helper functions.
 */

#include "helpers.h"

#include <QDirIterator>
#include <QStyleFactory>
#include <QStyleHints>
#include <QApplication>



/**
 * Returns a string containing the application's copyright information.
 * 
 * @return	The application's copyright information.
 */
QString getAppCopyrightString()
{
	return "2023-2024 Simon Vetter";
}

/**
 * Returns a string containing the link to the application's source code repository.
 * 
 * @return	The link to the application's source code repository.
 */
QString getAppCodeLink()
{
	return "https://github.com/svetter/pal";
}



/**
 * Returns a string containing the application's version number.
 * 
 * @return	The application's version number in string form.
 */
QString getAppVersion()
{
	return APP_VERSION;
}

/**
 * Compares two version strings and determines whether the first one is older than the second one.
 * 
 * @param version1	The first version string.
 * @param version2	The second version string.
 * @return			True if the first version is older than the second one, false otherwise.
 */
bool versionOlderThan(const QString& version1, const QString& version2)
{
	QStringList split1 = version1.split('.');
	QStringList split2 = version2.split('.');
	assert(split1.size() == 3 && split2.size() == 3);
	for (int i = 0; i < 3; i++) {
		bool conversionOk = false;
		int number1 = split1.at(i).toInt(&conversionOk);
		assert(conversionOk);
		int number2 = split2.at(i).toInt(&conversionOk);
		assert(conversionOk);
		if (number1 > number2) return false;
		if (number1 < number2) return true;
	}
	return false;
}

/**
 * Compares two version strings and determines whether the first one is older than or equal to the
 * second one.
 * 
 * @param version1	The first version string.
 * @param version2	The second version string.
 * @return			True if the first version is older than or equal to the second one, false otherwise.
 */
bool versionOlderOrEqual(const QString& version1, const QString& version2)
{
	return !versionOlderThan(version2, version1);
}





/**
 * Lists all languages supported by the application and returns two lists of the language codes and
 * the native language names, respectively.
 * 
 * @return	A pair of two lists, the first containing the language codes and the second containing the native language names.
 */
QPair<QStringList, QStringList> getSupportedLanguages()
{
	QStringList codes = QStringList("en");
	QStringList names = QStringList("English");
	
	QDirIterator it = QDirIterator(":/i18n/", QDirIterator::Subdirectories);
	while (it.hasNext()) {
		const QString code = it.next().split(".").at(0).split("/").at(2);
		if (code == "en") continue;
		codes.append(code);
		names.append(QLocale(code).nativeLanguageName());
	}
	
	return {codes, names};
}

/**
 * Determines which language should be used by default for the application, based on the user's
 * system language and the languages supported by the application.
 * 
 * @return	The language code of the language that should be used by default on the current system.
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
 * Lists all styles supported by the application and returns two lists of the style codes and the
 * translated style names, respectively.
 * 
 * @return	A pair of two lists, the first containing the style codes and the second containing the translated style names.
 */
QPair<QStringList, QStringList> getSupportedStyles()
{
	QStringList codes = QStyleFactory::keys();
	codes.insert(0, "");
	QStringList names = QStringList();
	
	for (const QString& code : codes) {
		     if (code == "")				names.append(QApplication::translate("General", "Default"));
		else if (code == "Fusion")			names.append(QApplication::translate("General", "Qt Fusion"));
		else if (code == "windows11")		names.append(QApplication::translate("General", "Windows 11"));
		else if (code == "windowsvista")	names.append(QApplication::translate("General", "Modern Windows"));
		else if (code == "Windows")			names.append(QApplication::translate("General", "Classic Windows"));
		else if (code == "macOS")			names.append(QApplication::translate("General", "MacOS"));
		else								names.append(code);
	}
	
	return {codes, names};
}



/**
 * Applies the given style to the application.
 * 
 * @param styleCode	The code of the style to apply.
 */
void applyStyle(QString styleCode)
{
	QApplication* application = qobject_cast<QApplication*>(QCoreApplication::instance());
	application->setStyle(styleCode);
}

/**
 * Applies the given color scheme to the application.
 * 
 * @param colorSchemeCode	The code of the color scheme to apply.
 */
void applyColorScheme(QString colorSchemeCode)
{
	Qt::ColorScheme colorScheme = Qt::ColorScheme::Unknown;
	if (colorSchemeCode.compare("light", Qt::CaseInsensitive) == 0) {
		colorScheme = Qt::ColorScheme::Light;
	} else if (colorSchemeCode.compare("dark", Qt::CaseInsensitive) == 0) {
		colorScheme = Qt::ColorScheme::Dark;
	}
	QGuiApplication::styleHints()->setColorScheme(colorScheme);
}





/**
 * Reverse of QString::toHtmlEscaped. Converts a string which was HTML-escaped by
 * QString::toHtmlEscaped back to its original form.
 * 
 * @param escapedString	The escaped string.
 * @return				The HTML-escaped string.
 */
QString fromHtmlEscaped(const QString& escapedString)
{
	/* QString::toHtmlEscaped() documentation:
	 * Converts a plain text string to an HTML string with HTML metacharacters <, >, &, and "
	 * replaced by HTML entities.
	 */
	QString stringCopy = escapedString;
	return stringCopy
		.replace("&lt;",	"<")
		.replace("&gt;",	">")
		.replace("&amp;",	"&")
		.replace("&quot;",	"\"");
}
