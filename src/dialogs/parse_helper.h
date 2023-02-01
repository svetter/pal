#ifndef PARSE_HELPER_H
#define PARSE_HELPER_H

#include <QString>
#include <QLineEdit>



QString parseLineedit(QLineEdit* lineedit) {
	QString raw = lineedit->text();
	if (raw.isEmpty()) {
		return QString();
	} else {
		return raw;
	}
}

// TODO



#endif // PARSE_HELPER_H
