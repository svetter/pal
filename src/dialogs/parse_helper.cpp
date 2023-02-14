#include "parse_helper.h"

#include "src/db/normal_table.h"



QString parseLineEdit(QLineEdit* lineEdit) {
	QString raw = lineEdit->text();
	if (raw.isEmpty()) {
		return QString();
	} else {
		return raw;
	}
}

QString parsePlainTextEdit(QPlainTextEdit* plainTextEdit)
{
	QString raw = plainTextEdit->toPlainText();
	if (raw.isEmpty()) {
		raw = QString();
	}
	return raw;
}

int parseSpinner(QSpinBox* spinBox)
{
	return spinBox->value();
}

int parseIDCombo(QComboBox* combo)
{
	return combo->currentData(NormalTable::PrimaryKeyRole).toInt();
}

int parseEnumCombo(QComboBox* combo, bool firstItemIsPlaceholder)
{
	int result = combo->currentIndex();
	if (result == 0 && firstItemIsPlaceholder) {
		result = -1;
	}
	return result;
}

bool parseCheckbox(QCheckBox* checkbox)
{
	return checkbox->isChecked();
}

QDate parseDateWidget(QDateEdit* dateEdit)
{
	return dateEdit->date();
}

QTime parseTimeWidget(QTimeEdit* timeEdit)
{
	return timeEdit->time();
}

QList<int> parseHikersList(QListView* hikersListWidget)
{
	// TODO #91
	return QList<int>();
}

QStringList parsePhotosList(QListView* photosListWidget)
{
	// TODO #91
	return QStringList();
}
