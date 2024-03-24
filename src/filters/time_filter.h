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
	virtual ~TimeFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static TimeFilter* decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
	friend class TimeFilterBox;
};



#endif // TIME_FILTER_H
