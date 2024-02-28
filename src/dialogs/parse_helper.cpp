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
 * @file parse_helper.cpp
 * 
 * This file defines functions to parse the values of widgets in item dialogs.
 */

#include "parse_helper.h"



/**
 * Parses a string from a line edit.
 * 
 * @param lineEdit	The line edit to parse.
 * @return			The parsed string.
 */
QString parseLineEdit(const QLineEdit& lineEdit) {
	QString raw = lineEdit.text();
	if (raw.isEmpty()) {
		return QString();
	} else {
		return raw;
	}
}

/**
 * Parses a string from a plain text edit.
 * 
 * @param plainTextEdit	The plain text edit to parse.
 * @return				The parsed string.
 */
QString parsePlainTextEdit(const QPlainTextEdit& plainTextEdit)
{
	QString raw = plainTextEdit.toPlainText();
	if (raw.isEmpty()) {
		raw = QString();
	}
	return raw;
}

/**
 * Parses an int from a spinner.
 * 
 * @param spinBox	The spinner to parse.
 * @return			The parsed int.
 */
int parseSpinner(const QSpinBox& spinBox)
{
	return spinBox.value();
}

/**
 * Parses an ItemID from an item combo box.
 * 
 * @param combo		The combo box containing items to parse.
 * @param itemIDs	The list of item IDs corresponding to the entries in the combo box.
 * @return			The parsed ItemID, which may be invalid.
 */
ItemID parseItemCombo(const QComboBox& combo, const QList<ValidItemID>& itemIDs)
{
	if (combo.currentIndex() < 1) {
		return ItemID();
	}
	return itemIDs.at(combo.currentIndex() - 1);
}

/**
 * Parses an int from an enumerative combo box.
 * 
 * @param combo			The combo box containing entries from an enumerator to parse.
 * @param defaultToZero	Whether to return 0 if there is no valid selection.
 * @return				The parsed row index, which may be invalid.
 */
int parseEnumCombo(const QComboBox& combo, bool defaultToZero)
{
	if (defaultToZero && combo.currentIndex() < 0) return 0;
	return combo.currentIndex();
}

/**
 * Parses a bool from a checkbox.
 * 
 * @param checkbox	The checkbox to parse.
 * @return			True if the checkbox is checked, false otherwise.
 */
bool parseCheckbox(const QCheckBox& checkbox)
{
	return checkbox.isChecked();
}

/**
 * Parses a date from a date edit.
 * 
 * @param dateEdit	The date edit to parse.
 * @return			The parsed date.
 */
QDate parseDateWidget(const QDateEdit& dateEdit)
{
	return dateEdit.date();
}

/**
 * Parses a time from a time edit.
 * 
 * @param timeEdit	The time edit to parse.
 * @return			The parsed time.
 */
QTime parseTimeWidget(const QTimeEdit& timeEdit)
{
	return timeEdit.time();
}
