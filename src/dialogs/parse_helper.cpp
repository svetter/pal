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

ItemID parseIDCombo(const QComboBox* combo)
{
	int primaryKey = combo->currentData(NormalTable::PrimaryKeyRole).toInt();
	return ItemID(primaryKey);
}

int parseEnumCombo(const QComboBox* combo)
{
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
