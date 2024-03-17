#ifndef ENUM_FILTER_BOX_H
#define ENUM_FILTER_BOX_H

#include "src/main/filters/filter_box.h"

#include <QComboBox>



class EnumFilterBox : public FilterBox
{
	Q_OBJECT
	
	QComboBox* combo;
	const QStringList entries;
	
public:
	explicit EnumFilterBox(QWidget* parent, const QString& title, const QStringList& entries);
	virtual ~EnumFilterBox();
	
	virtual void setup();
	virtual void reset();
};



#endif // ENUM_FILTER_BOX_H
