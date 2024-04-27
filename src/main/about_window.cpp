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
 * @file about_window.cpp
 * 
 * This file defines the class controlling the About window.
 */

#include "about_window.h"

#include "src/main/helpers.h"

#include <QtSvg>
#include <QSvgWidget>



/**
 * Creates a new About window.
 * 
 * @param parent	Parent widget.
 */
AboutWindow::AboutWindow(QWidget& parent) :
	QDialog(&parent)
{
	setupUi(this);
	
	QString appVersion	= getAppVersion();
	QString qtVersion	= QString("%1.%2.%3").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR).arg(QT_VERSION_PATCH);
	
	appNameLabel	->setText(appNameLabel	->text().arg(appVersion));
	copyrightLabel	->setText(copyrightLabel->text().arg(QStringLiteral(APP_COPYRIGHT).toHtmlEscaped()));
	codeLabel		->setText(codeLabel		->text().arg(QStringLiteral(CODE_LINK).toHtmlEscaped()));
	qtInfoLabel		->setText(qtInfoLabel	->text().arg(qtVersion));
}
