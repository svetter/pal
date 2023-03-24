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
