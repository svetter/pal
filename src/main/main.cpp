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
