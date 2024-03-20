#ifndef TIME_FILTER_H
#define TIME_FILTER_H

#include "filter.h"

#include <QTime>



class TimeFilter : public Filter
{
	QTime min;
	QTime max;
	
public:
	TimeFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValues(const QTime& min, const QTime& max);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // TIME_FILTER_H
