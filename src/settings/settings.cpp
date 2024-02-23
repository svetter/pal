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
 * @file settings.cpp
 * 
 * This file defines part of the Settings class.
 */

#include "settings.h"

#include <QScreen>



/**
 * Determines whether the settings were last saved by a version of the application that is older
 * than or equal to the given version.
 * 
 * @param version	The version of the application with which to compare the settings version.
 * @return			True if the settings version is below or equal to the given version, false otherwise.
 */
bool Settings::settingsVersionUpTo(const QString& version)
{
	return versionOlderOrEqual(appVersion.get(), version);
}



/**
 * Updates the settings file to the current version of the application.
 * 
 * In this function, it is important to use string literals for all keys, since the update process
 * should still work in the future when keys will potentially change.
 */
void Settings::checkForVersionChange()
{
	// 1.0.3 or older
	if (settingsVersionUpTo("1.0.3")) {
		// New columns => reset column widths
		qSettings.remove("implicit/mainWindow/columnWidths");
	}
	
	// 1.1.1. or older
	if (settingsVersionUpTo("1.1.1")) {
		// Open tab, filter bar visibility, column widths, sortings moved to project settings => remove
		qSettings.remove("implicit/mainWindow/currentTabIndex");
		qSettings.remove("implicit/mainWindow/showFilters");
		qSettings.remove("implicit/mainWindow/columnWidths");
		qSettings.remove("implicit/mainWindow/sorting");
		
		// New splitter in ascent viewer => remove deprecated setting
		qSettings.remove("implicit/ascentViewer/splitterSizes");

		// Inconsistent key changed => move to new key
		QString oldCountryDialogGeometryKey = "implicit/country/geometry";
		QString newCountryDialogGeometryKey = "implicit/countryDialog/geometry";
		if (qSettings.contains(oldCountryDialogGeometryKey)) {
			QVariant value = qSettings.value(oldCountryDialogGeometryKey);
			qSettings.setValue(newCountryDialogGeometryKey, value);
			qSettings.remove(oldCountryDialogGeometryKey);
		}
	}
	
	// 1.2.1 or older
	if (settingsVersionUpTo("1.2.1")) {
		// Geometry saving and restoring fixed/changed => reset
		resetGeometrySettings();
	}
	
	// Update settings version
	const QString currentAppVersion = getAppVersion();
	const QString currentSettingsVersion = appVersion.get();
	if (versionOlderThan(currentSettingsVersion, currentAppVersion)) {
		qDebug().noquote().nospace() << "Upgraded global settings from v" << currentSettingsVersion << " to v" << currentAppVersion;
		appVersion.set(currentAppVersion);
	}
}





/**
 * Stores implicit settings about position and geometry for the given dialog.
 * 
 * @param dialog			The dialog for which to store the geometry.
 * @param mainWindow		The application's main window (for determining relative position).
 * @param geometrySetting	The geometry setting corresponding to the dialog.
 */
void saveDialogGeometry(QWidget* dialog, QMainWindow* mainWindow, const Setting<QRect>* geometrySetting)
{
	QRect absoluteGeometry = dialog->geometry();
	if (Settings::rememberWindowPositionsRelative.get()) {
		absoluteGeometry.translate(- mainWindow->pos());
	}
	geometrySetting->set(absoluteGeometry);
}

/**
 * Restores implicit settings about position and geometry for the given dialog.
 * 
 * @param dialog			The dialog for which to restore the geometry.
 * @param mainWindow		The application's main window (for restoring relative position).
 * @param geometrySetting	The geometry setting corresponding to the dialog.
 */
void restoreDialogGeometry(QWidget* dialog, QMainWindow* mainWindow, const Setting<QRect>* geometrySetting)
{
	if (!Settings::rememberWindowPositions.present()) return;
	
	QRect savedGeometry = geometrySetting->get();
	if (savedGeometry.isEmpty()) return;
	
	if (Settings::rememberWindowPositionsRelative.get()) {
		savedGeometry.translate(mainWindow->pos());
		
		// Change size if bigger than screen
		const QScreen* screen = QGuiApplication::screenAt(savedGeometry.center());
		if (!screen) screen = mainWindow->screen();
		assert(screen);
		QRect screenGeometry = screen->availableGeometry();
		const int frameTopHeight = 30;	// Extra space for window top bar
		screenGeometry.setTop(screenGeometry.top() + frameTopHeight);
		if (savedGeometry.width () > screenGeometry.width ())	savedGeometry.setWidth  (screenGeometry.width ());
		if (savedGeometry.height() > screenGeometry.height())	savedGeometry.setHeight (screenGeometry.height());
		
		// Move if (partly) out of bounds
		if (savedGeometry.left  () < screenGeometry.left  ())	savedGeometry.moveLeft  (screenGeometry.left  ());
		if (savedGeometry.right () > screenGeometry.right ())	savedGeometry.moveRight (screenGeometry.right ());
		if (savedGeometry.top   () < screenGeometry.top   ())	savedGeometry.moveTop   (screenGeometry.top   ());
		if (savedGeometry.bottom() > screenGeometry.bottom())	savedGeometry.moveBottom(screenGeometry.bottom());
	}
	
	dialog->setGeometry(savedGeometry);
}



/**
 * Saves the current sizes of the given splitter to the given setting.
 * 
 * @param splitter				The splitter to save the sizes of.
 * @param splitterSizesSetting	The setting to save the splitter sizes to.
 */
void saveSplitterSizes(QSplitter* splitter, const Setting<QStringList>* splitterSizesSetting)
{
	QList<int> leftSplitterSizes = splitter->sizes();
	QStringList stringList;
	for (int size : leftSplitterSizes) {
		stringList.append(QString::number(size));
	}
	splitterSizesSetting->set(stringList);
}

/**
 * Restores the sizes of all splitters from settings.
 * 
 * @param splitter				The splitter to restore the sizes of.
 * @param splitterSizesSetting	The setting to load the splitter sizes from.
 */
void restoreSplitterSizes(QSplitter* splitter, const Setting<QStringList>* splitterSizesSetting)
{
	QStringList splitterSizeStrings = splitterSizesSetting->get();
	if (splitterSizeStrings.size() != splitter->sizes().size()) {
		// Can't restore splitter sizes from settings
		if (!splitterSizeStrings.isEmpty()) {
			qDebug() << QString("Couldn't restore splitter sizes for ascent viewer: Expected %1 numbers, but got %2")
							.arg(splitter->sizes().size()).arg(splitterSizeStrings.size());
		}
		splitterSizesSetting->clear();
		return;
	}
	
	QList<int> splitterSizes = QList<int>();
	for (const QString& sizeString : splitterSizeStrings) {
		bool conversionOk = false;
		int size = sizeString.toInt(&conversionOk);
		if (!conversionOk) {
			qDebug() << QString("Couldn't restore splitter sizes for ascent viewer: Value(s) couldn't be converted to int");
			splitterSizesSetting->clear();
			return;
		}
		splitterSizes.append(size);
	}
	
	splitter->setSizes(splitterSizes);
}
