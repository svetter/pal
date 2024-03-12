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
 * @file startup_test.cpp
 * 
 * This file defines the startup test cases.
 */

#include <QCoreApplication>
#include <QtTest>

#include "src/main/about_window.h"
#include "src/main/main_window.h"
#include "src/settings/settings_window.h"



/**
 * Test whether the application starts up and shuts down without crashing.
 */
class StartupTest : public QObject
{
	Q_OBJECT
	
	MainWindow* mainWindow;
	
	
	
private slots:
	void initTestCase()
	{
		mainWindow = new MainWindow();
		wait();
	}
	
	void testStartup()
	{
		QVERIFY(mainWindow->isVisible() || QGuiApplication::platformName() == "offscreen");
		mainWindow->findChild<QAction*>("closeDatabaseAction")->trigger();
		wait();
	}
	
	void testSettingsWindow()
	{
		mainWindow->findChild<QAction*>("settingsAction")->trigger();
		wait();
		QTest::keyClick(mainWindow->findChild<SettingsWindow*>("SettingsWindow"), Qt::Key_Escape);
		wait();
	}
	
	void testAboutWindow()
	{
		mainWindow->findChild<QAction*>("aboutAction")->trigger();
		wait();
		QTest::keyClick(mainWindow->findChild<AboutWindow*>("AboutWindow"), Qt::Key_Escape);
		wait();
	}
	
	void cleanupTestCase()
	{
		delete mainWindow;
	}
	
	
	
private:
	void wait()
	{
		QTest::qWait(200);
	}
};



QTEST_MAIN(StartupTest)
#include "startup_test.moc"
