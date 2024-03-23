#ifndef TIME_FILTER_BOX_H
#define TIME_FILTER_BOX_H

#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/time_filter.h"

#include <QTimeEdit>
#include <QCheckBox>



class TimeFilterBox : public FilterBox
{
	Q_OBJECT
	
	TimeFilter& filter;
	
	QTimeEdit* minTimeWidget;
	QTimeEdit* maxTimeWidget;
	QCheckBox* setMaxTimeCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit TimeFilterBox(QWidget* parent, const QString& title, TimeFilter& filter);
	virtual ~TimeFilterBox();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
	
private slots:
	void handle_minTimeChanged();
	void handle_maxTimeChanged();
	void handle_setMaxTimeChanged();
};



#endif // TIME_FILTER_BOX_H
