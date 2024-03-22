#ifndef TIME_FILTER_BOX_H
#define TIME_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/time_filter.h"

#include <QTimeEdit>
#include <QCheckBox>



class TimeFilterBox : public FilterBox
{
	Q_OBJECT
	
	unique_ptr<TimeFilter> filter;
	
	QTimeEdit* minTimeWidget;
	QTimeEdit* maxTimeWidget;
	QCheckBox* setMaxTimeCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit TimeFilterBox(QWidget* parent, const QString& title, unique_ptr<TimeFilter> filter);
	virtual ~TimeFilterBox();
	
	virtual void setup();
	virtual void reset();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter* getFilter() const;
protected:
	virtual Filter* getFilter();
	
private slots:
	void handle_minTimeChanged();
	void handle_maxTimeChanged();
	void handle_setMaxTimeChanged();
};



#endif // TIME_FILTER_BOX_H
