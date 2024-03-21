#ifndef INT_FILTER_BOX_H
#define INT_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/int_filter.h"

#include <QSpinBox>
#include <QCheckBox>



class IntFilterBox : public FilterBox
{
	Q_OBJECT
	
	unique_ptr<IntFilter> filter;
	
	QSpinBox* minSpinner;
	QSpinBox* maxSpinner;
	QCheckBox* setMaxCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit IntFilterBox(QWidget* parent, const QString& title, unique_ptr<IntFilter> filter);
	virtual ~IntFilterBox();
	
	virtual void setup();
	virtual void reset();
	
	virtual const Filter* getFilter() const;
	
private slots:
	void handle_minChanged();
	void handle_maxChanged();
	void handle_setMaxChanged();
};



#endif // INT_FILTER_BOX_H
