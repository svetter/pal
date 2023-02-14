#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

#include <QString>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QListWidget>
#include <QPlainTextEdit>



QString		parseLineEdit		(QLineEdit*			lineEdit);
QString		parsePlainTextEdit	(QPlainTextEdit*	plainTextEdit);
int			parseSpinner		(QSpinBox*			spinBox);
int			parseIDCombo		(QComboBox*			combo);
int			parseEnumCombo		(QComboBox*			combo, bool firstItemIsPlaceholder);
bool		parseCheckbox		(QCheckBox*			checkbox);
QDate		parseDateWidget		(QDateEdit*			dateEdit);
QTime		parseTimeWidget		(QTimeEdit*			timeEdit);
QList<int>	parseHikerList		(QListWidget*		hikersListWidget);
QStringList	parsePhotosList		(QListWidget*		photosListWidget);



#endif // PARSE_HELPER_H
