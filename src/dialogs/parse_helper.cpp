/*
 * Copyright 2023 Simon Vetter
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

#include "parse_helper.h"



QString parseLineEdit(const QLineEdit* lineEdit) {
	QString raw = lineEdit->text();
	if (raw.isEmpty()) {
		return QString();
	} else {
		return raw;
	}
}

QString parsePlainTextEdit(const QPlainTextEdit* plainTextEdit)
{
	QString raw = plainTextEdit->toPlainText();
	if (raw.isEmpty()) {
		raw = QString();
	}
	return raw;
}

int parseSpinner(const QSpinBox* spinBox)
{
	return spinBox->value();
}

ItemID parseItemCombo(const QComboBox* combo, const QList<ValidItemID>& itemIDs)
{
	if (combo->currentIndex() < 1) {
		return ItemID();
	}
	return itemIDs.at(combo->currentIndex() - 1);
}

int parseEnumCombo(const QComboBox* combo, bool defaultToZero)
{
	if (defaultToZero && combo->currentIndex() < 0) return 0;
	return combo->currentIndex();
}

bool parseCheckbox(const QCheckBox* checkbox)
{
	return checkbox->isChecked();
}

QDate parseDateWidget(const QDateEdit* dateEdit)
{
	return dateEdit->date();
}

QTime parseTimeWidget(const QTimeEdit* timeEdit)
{
	return timeEdit->time();
}
