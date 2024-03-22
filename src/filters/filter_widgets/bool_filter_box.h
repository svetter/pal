#ifndef BOOL_FILTER_BOX_H
#define BOOL_FILTER_BOX_H

#include "src/filters/bool_filter.h"
#include "src/filters/filter_widgets/filter_box.h"

#include <QCheckBox>



class BoolFilterBox : public FilterBox
{
	Q_OBJECT
	
	unique_ptr<BoolFilter> filter;
	
	QRadioButton* yesRadiobutton;
	QRadioButton* noRadiobutton;
	QButtonGroup yesNoButtonGroup;
	QSpacerItem* spacerL;
	QSpacerItem* spacerR;
	
public:
	explicit BoolFilterBox(QWidget* parent, const QString& title, unique_ptr<BoolFilter> filter);
	virtual ~BoolFilterBox();
	
	virtual void setup();
	virtual void reset();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter* getFilter() const;
protected:
	virtual Filter* getFilter();
};



#endif // BOOL_FILTER_BOX_H
