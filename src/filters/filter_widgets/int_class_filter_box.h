#ifndef INT_CLASS_FILTER_BOX_H
#define INT_CLASS_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/int_filter.h"

#include <QComboBox>
#include <QCheckBox>



class IntClassFilterBox : public FilterBox
{
	Q_OBJECT
	
	IntFilter& filter;
	
	const int classIncrement;
	const int minValue;
	const int maxValue;
	
	QComboBox* minCombo;
	QComboBox* maxCombo;
	QCheckBox* setMaxCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit IntClassFilterBox(QWidget* parent, const QString& title, int classIncrement, int minValue, int maxValue, IntFilter& filter);
	virtual ~IntClassFilterBox();
	
	virtual void setup();
	virtual void reset();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
	
private slots:
	void handle_minChanged();
	void handle_maxChanged();
	void handle_setMaxChanged();
};



#endif // INT_CLASS_FILTER_BOX_H
