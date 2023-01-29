#include "mainwindow.h"

#include <iostream>
#include <QApplication>
#include <QLocale>
#include <QTranslator>

using namespace std;



int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	
	
	
	QString defaultLanguage = "en";
	bool useTranslator = true;
	bool forceTranslation = false;
	
	QTranslator translator;
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	if (uiLanguages.contains(defaultLanguage)) {
		useTranslator = false;
		cout << "No need for translation, system lists default language " << defaultLanguage.toStdString() << endl;
	}
	useTranslator |= forceTranslation;
	
	for (const QString &locale : uiLanguages) {
		const QString baseName = "PAL_" + QLocale(locale).name();
		if (translator.load(baseName, ":/i18n/")) {
			cout << "Found translation for " << locale.toStdString() << endl;
			if (translator.isEmpty()) {
				cout << "Translator empty!" << endl;
			}
			if (useTranslator) {
				application.installTranslator(&translator);
			}
			break;
		}
		else {
			cout << "Found NO translation for system language " << locale.toStdString() << endl;
		}
	}
	
	
	
	MainWindow w;
	w.show();
	return application.exec();
}
