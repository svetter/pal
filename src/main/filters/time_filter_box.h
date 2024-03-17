#ifndef TIME_FILTER_BOX_H
#define TIME_FILTER_BOX_H

#include "src/main/filters/filter_box.h"

#include <QTimeEdit>
#include <QCheckBox>



class TimeFilterBox : public FilterBox
{
	Q_OBJECT
	
	QTimeEdit* minTimeWidget;
	QTimeEdit* maxTimeWidget;
	QCheckBox* setMaxTimeCheckbox;
	QSpacerItem* spacer;
	
public:
	explicit TimeFilterBox(QWidget* parent, const QString& title);
	virtual ~TimeFilterBox();
	
	virtual void setup();
	virtual void reset();
	
private slots:
	void handle_minTimeChanged();
	void handle_maxTimeChanged();
	void handle_setMaxTimeChanged();
};



#endif // TIME_FILTER_BOX_H
