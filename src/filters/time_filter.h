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
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual unique_ptr<FilterBox> createFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static unique_ptr<TimeFilter> decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
};



#endif // TIME_FILTER_H
