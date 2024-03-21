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
	
	void setMinMax(const QDate& min, const QDate& max);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const;
	static unique_ptr<DateFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // DATE_FILTER_H
