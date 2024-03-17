#ifndef INT_CLASS_FILTER_BOX_H
#define INT_CLASS_FILTER_BOX_H

#include "src/main/filters/filter_box.h"

#include <QComboBox>
#include <QCheckBox>



class IntClassFilterBox : public FilterBox
{
	Q_OBJECT
	
	const int classIncrement;
	const int minValue;
	const int maxValue;
	
	QComboBox* minCombo;
	QComboBox* maxCombo;
	QCheckBox* setMaxCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit IntClassFilterBox(QWidget* parent, const QString& title, int classIncrement, int minValue, int maxValue);
	virtual ~IntClassFilterBox();
	
	virtual void setup();
	virtual void reset();
	
private slots:
	void handle_minChanged();
	void handle_maxChanged();
	void handle_setMaxChanged();
};



#endif // INT_CLASS_FILTER_BOX_H
