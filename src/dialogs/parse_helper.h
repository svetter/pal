#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

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
int			parseIDCombo		(const QComboBox*		combo);
int			parseEnumCombo		(const QComboBox*		combo, bool firstItemIsPlaceholder);
bool		parseCheckbox		(const QCheckBox*		checkbox);
QDate		parseDateWidget		(const QDateEdit*		dateEdit);
QTime		parseTimeWidget		(const QTimeEdit*		timeEdit);



#endif // PARSE_HELPER_H
