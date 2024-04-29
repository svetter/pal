#include "time_filter.h"

#include "src/filters/filter_widgets/time_filter_box.h"



TimeFilter::TimeFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(Time, tableToFilter, columnToFilterBy, uiName),
	min(QTime(12, 0)),
	max(QTime(12, 0))
{}

TimeFilter::~TimeFilter()
{}



bool TimeFilter::evaluate(const QVariant& rawRowValue) const
{
	assert(!min.isNull());
	assert(!max.isNull());
	
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<QTime>() && rawRowValue.toTime().isValid());
		const QTime convertedValue = rawRowValue.toTime();
		const bool match = convertedValue >= min && convertedValue <= max;
		
		return match != isInverted();
	}
}



FilterBox* TimeFilter::createFilterBox(QWidget* parent)
{
	return new TimeFilterBox(parent, uiName, *this);
}



QStringList TimeFilter::encodeTypeSpecific() const
{
	return {
		encodeTime("min", min),
		encodeTime("max", max)
	};
}

TimeFilter* TimeFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const QTime min = decodeTime(restOfEncoding, "min", ok);
	if (!ok) return nullptr;
	const QTime max = decodeTime(restOfEncoding, "max", ok);
	if (!ok) return nullptr;
	
	TimeFilter* const filter = new TimeFilter(tableToFilter, columnToFilterBy, uiName);
	filter->min = min;
	filter->max = max;
	
	return filter;
}
