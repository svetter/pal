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
