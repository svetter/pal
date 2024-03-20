#ifndef DUAL_ENUM_FILTER_BOX_H
#define DUAL_ENUM_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"

#include <QComboBox>



class DualEnumFilterBox : public FilterBox
{
	Q_OBJECT
	
	QComboBox* comboDiscerning;
	QComboBox* comboDependent;
	const QList<QPair<QString, QStringList>> entries;
	
public:
	explicit DualEnumFilterBox(QWidget* parent, const QString& title, const QList<QPair<QString, QStringList>>& entries);
	virtual ~DualEnumFilterBox();
	
	virtual void setup();
	virtual void reset();
	
private slots:
	void handle_discerningComboChanged();
};



#endif // DUAL_ENUM_FILTER_BOX_H
