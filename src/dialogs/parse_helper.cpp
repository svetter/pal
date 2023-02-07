#include "parse_helper.h"



QString parseLineEdit(QLineEdit* lineEdit) {
	QString raw = lineEdit->text();
	if (raw.isEmpty()) {
		return QString();
	} else {
		return raw;
	}
}



QString parsePlainTextEdit (QPlainTextEdit* plainTextEdit)
{
	QString raw = plainTextEdit->toPlainText();
	if (raw.isEmpty()) {
		raw = QString();
	}
	return raw;
}



int parseSpinner (QSpinBox* spinBox)
{
	return spinBox->value();
}



int parseIDCombo (QComboBox* combo)
{
	// TODO
}



int parseEnumCombo (QComboBox* combo)
{
	return combo->currentIndex();
}



bool parseCheckbox (QCheckBox* checkbox)
{
	return checkbox->isChecked();
}



QDate parseDateWidget (QDateEdit* dateEdit)
{
	return dateEdit->date();
}



QTime parseTimeWidget (QTimeEdit* timeEdit)
{
	return timeEdit->time();
}



QList<int> parseHikerList (QListWidget* hikersListWidget)
{
	// TODO
}



QList<QString> parsePhotosList (QListWidget* photosListWidget)
{
	// TODO
}