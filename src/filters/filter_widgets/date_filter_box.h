#ifndef DATE_FILTER_BOX_H
#define DATE_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"

#include <QDateEdit>
#include <QCheckBox>



class DateFilterBox : public FilterBox
{
	Q_OBJECT
	
	QDateEdit* minDateWidget;
	QDateEdit* maxDateWidget;
	QCheckBox* setMaxDateCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit DateFilterBox(QWidget* parent, const QString& title);
	virtual ~DateFilterBox();
	
	virtual void setup();
	virtual void reset();
	
private slots:
	void handle_minDateChanged();
	void handle_maxDateChanged();
	void handle_setMaxDateChanged();
};



#endif // DATE_FILTER_BOX_H
