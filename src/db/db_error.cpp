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
 * @file db_error.h
 * 
 * This file defines functions for displaying database errors.
 */

#include "db_error.h"

#include "database.h"

#include <QMessageBox>



/**
 * Displays an error message.
 * 
 * @param parent	The parent window.
 * @param error		The error message.
 */
void displayError(QWidget& parent, QString error)
{
	QMessageBox::critical(&parent, Database::tr("Database error"), error);
	exit(1);
}

/**
 * Translates and formats a QSqlError into a string to use in an error message.
 *
 * @param error	The error.
 * @return		A formatted and translated string representing the error.
 */
QString formatSqlError(QSqlError error)
{
	QString driverError		= error.driverText();	// Translated
	QString databaseError	= error.databaseText();	// Untranslated
	return driverError + ".\n\n" + Database::tr("Details:") + " \"" + databaseError + "\"";
}



/**
 * Displays an error message including the query that caused the error.
 *
 * @param parent		The parent window.
 * @param error			The error message.
 * @param queryString	The query that caused the error.
 */
void displayError(QWidget& parent, QString error, QString& queryString)
{
	return displayError(parent, error + "\n\n" + Database::tr("Query:") + "\n" + queryString);
}

/**
 * Displays a QSqlError as an error message, along with the query that caused the error.
 *
 * @param parent		The parent window.
 * @param error			The error.
 * @param queryString	The query that caused the error.
 */
void displayError(QWidget& parent, QSqlError error, QString& queryString)
{
	return displayError(parent, formatSqlError(error), queryString);
}

/**
 * Displays a QSqlError as an error message.
 *
 * @param parent	The parent window.
 * @param error		The error.
 */
void displayError(QWidget& parent, QSqlError error)
{
	return displayError(parent, formatSqlError(error));
}
