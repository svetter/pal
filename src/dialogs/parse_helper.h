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
 * @file parse_helper.h
 * 
 * This file declares functions to parse the values of widgets in item dialogs.
 */

#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

#include "src/data/item_id.h"

#include <QString>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QListView>
#include <QPlainTextEdit>



QString		parseLineEdit		(const QLineEdit*		lineEdit);
QString		parsePlainTextEdit	(const QPlainTextEdit*	plainTextEdit);
int			parseSpinner		(const QSpinBox*		spinBox);
ItemID		parseItemCombo		(const QComboBox*		combo, const QList<ValidItemID>& itemIDs);
int			parseEnumCombo		(const QComboBox*		combo, bool defaultToZero);
bool		parseCheckbox		(const QCheckBox*		checkbox);
QDate		parseDateWidget		(const QDateEdit*		dateEdit);
QTime		parseTimeWidget		(const QTimeEdit*		timeEdit);



#endif // PARSE_HELPER_H
