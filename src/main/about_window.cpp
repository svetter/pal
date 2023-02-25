#include "about_window.h"

#include <QtSvg>
#include <QSvgWidget>



AboutWindow::AboutWindow(QWidget* parent) :
		QDialog(parent)
{
	setupUi(this);
	
	QString qtVersion = QString::number(QT_VERSION_MAJOR) + "." + QString::number(QT_VERSION_MINOR) + "." + QString::number(QT_VERSION_PATCH);
	appNameLabel	->setText(appNameLabel	->text().replace("$APP_VERSION$",	QStringLiteral(APP_VERSION	).toHtmlEscaped()));
	copyrightLabel	->setText(copyrightLabel->text().replace("$APP_COPYRIGHT$",	QStringLiteral(APP_COPYRIGHT).toHtmlEscaped()));
	codeLabel		->setText(codeLabel		->text().replace("$CODE_LINK$",		QStringLiteral(CODE_LINK	).toHtmlEscaped()));
	qtInfoLabel		->setText(qtInfoLabel	->text().replace("$QT_VERSION$",	qtVersion));
}
