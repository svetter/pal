#include "parse_helper.h"

#include "src/db/normal_table.h"



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

int parseIDCombo(const QComboBox* combo)
{
	return combo->currentData(NormalTable::PrimaryKeyRole).toInt();
}

int parseEnumCombo(const QComboBox* combo, bool firstItemIsPlaceholder)
{
	int result = combo->currentIndex();
	if (result == 0 && firstItemIsPlaceholder) {
		result = -1;
	}
	return result;
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
