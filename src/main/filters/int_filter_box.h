#ifndef INT_FILTER_BOX_H
#define INT_FILTER_BOX_H

#include "src/main/filters/filter_box.h"

#include <QSpinBox>
#include <QCheckBox>



class IntFilterBox : public FilterBox
{
	Q_OBJECT
	
	QSpinBox* minSpinner;
	QSpinBox* maxSpinner;
	QCheckBox* setMaxCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit IntFilterBox(QWidget* parent, const QString& title);
	virtual ~IntFilterBox();
	
	virtual void setup();
	virtual void reset();
	
private slots:
	void handle_minChanged();
	void handle_maxChanged();
	void handle_setMaxChanged();
};



#endif // INT_FILTER_BOX_H
