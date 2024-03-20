#ifndef BOOL_FILTER_BOX_H
#define BOOL_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"

#include <QCheckBox>



class BoolFilterBox : public FilterBox
{
	Q_OBJECT
	
	QRadioButton* yesRadiobutton;
	QRadioButton* noRadiobutton;
	QSpacerItem* spacerL;
	QSpacerItem* spacerR;
	
public:
	explicit BoolFilterBox(QWidget* parent, const QString& title);
	virtual ~BoolFilterBox();
	
	virtual void setup();
	virtual void reset();
	
private slots:
	void handle_radiobuttonClicked();
};



#endif // BOOL_FILTER_BOX_H
