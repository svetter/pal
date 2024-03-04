#include <QCoreApplication>
#include <QtTest>

#include "src/main/main_window.h"



/**
 * Test whether the application starts up and shuts down without crashing.
 */
class StartupTest : public QObject
{
	Q_OBJECT
	
private slots:
	/**
	 * Test whether the application starts up and shuts down without crashing.
	 */
	void testStartup()
	{
		MainWindow mainWindow = MainWindow();
		
		QTest::qWait(1000);
		QTest::keySequence(&mainWindow, QKeySequence(Qt::Key_Alt + Qt::Key_F4));
	}
};



QTEST_MAIN(StartupTest)
#include "startup_test.moc"
