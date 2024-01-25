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
 * This file declares functions for displaying database errors.
 */

#ifndef DB_ERROR_H
#define DB_ERROR_H

#include <QWidget>
#include <QString>
#include <QSqlError>



void displayError(QWidget* parent, QString error);
QString formatSqlError(QSqlError error);

void displayError(QWidget* parent, QString error, QString& queryString);
void displayError(QWidget* parent, QSqlError error);
void displayError(QWidget* parent, QSqlError error, QString& queryString);



#endif // DB_ERROR_H
