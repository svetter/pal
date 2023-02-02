#include "main_window.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>



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
		qDebug() << "No need for translation, system lists default language " << defaultLanguage;
	}
	useTranslator |= forceTranslation;
	
	for (const QString &locale : uiLanguages) {
		const QString baseName = "PAL_" + QLocale(locale).name();
		if (translator.load(baseName, ":/i18n/")) {
			qDebug() << "Found translation for " + locale;
			if (translator.isEmpty()) {
				qDebug() << "Translator empty!";
			}
			if (useTranslator) {
				application.installTranslator(&translator);
			}
			break;
		}
		else {
			qDebug() << "Found NO translation for system language " << locale;
		}
	}
	
	
	
	MainWindow w;
	w.show();
	return application.exec();
}
