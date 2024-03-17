#ifndef STRING_FILTER_BOX_H
#define STRING_FILTER_BOX_H

#include "src/main/filters/filter_box.h"

#include <QLineEdit>



class StringFilterBox : public FilterBox
{
	Q_OBJECT
	
	QLineEdit* lineEdit;
	
public:
	explicit StringFilterBox(QWidget* parent, const QString& title);
	virtual ~StringFilterBox();
	
	virtual void setup();
	virtual void reset();
};



#endif // STRING_FILTER_BOX_H
