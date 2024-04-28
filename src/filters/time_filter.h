#ifndef TIME_FILTER_H
#define TIME_FILTER_H

#include "filter.h"

#include <QTime>



class TimeFilter : public Filter
{
	QTime min;
	QTime max;
	
public:
	TimeFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
	virtual ~TimeFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static TimeFilter* decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding);
	
	friend class Filter;
	friend class TimeFilterBox;
};



#endif // TIME_FILTER_H
