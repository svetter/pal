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
	
	void setMinMax(const QTime& min, const QTime& max);
	
	virtual unique_ptr<FilterBox> getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const;
	
protected:
	virtual QStringList encodeTypeSpecific() const;
	static unique_ptr<TimeFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // TIME_FILTER_H
