#ifndef DATE_FILTER_H
#define DATE_FILTER_H

#include "filter.h"

#include <QDate>



class DateFilter : public Filter
{
	QDate min;
	QDate max;
	
public:
	DateFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
	void setValues(const QDate& min, const QDate& max);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const;
};



#endif // DATE_FILTER_H
