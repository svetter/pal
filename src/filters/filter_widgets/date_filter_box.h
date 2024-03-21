#ifndef DATE_FILTER_BOX_H
#define DATE_FILTER_BOX_H

#include "src/filters/date_filter.h"
#include "src/filters/filter_widgets/filter_box.h"

#include <QDateEdit>
#include <QCheckBox>



class DateFilterBox : public FilterBox
{
	Q_OBJECT
	
	unique_ptr<DateFilter> filter;
	
	QDateEdit* minDateWidget;
	QDateEdit* maxDateWidget;
	QCheckBox* setMaxDateCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit DateFilterBox(QWidget* parent, const QString& title, unique_ptr<DateFilter> filter);
	virtual ~DateFilterBox();
	
	virtual void setup();
	virtual void reset();
	
	virtual const Filter* getFilter() const;
	
private slots:
	void handle_minDateChanged();
	void handle_maxDateChanged();
	void handle_setMaxDateChanged();
};



#endif // DATE_FILTER_BOX_H
