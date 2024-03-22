#include "time_filter.h"

#include "src/filters/filter_widgets/time_filter_box.h"



TimeFilter::TimeFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Time, tableToFilter, columnToFilterBy, FilterFoldOp(-1), name),
	min(QTime()),
	max(QTime())
{}



void TimeFilter::setMinMax(const QTime& min, const QTime& max)
{
	this->min = min;
	this->max = max;
}



bool TimeFilter::evaluate(const QVariant& rawRowValue) const
{
	assert(!min.isNull());
	assert(!max.isNull());
	
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<QTime>());
		const QTime convertedValue = rawRowValue.toTime();
		const bool match = convertedValue >= min && convertedValue <= max;
		
		return match != isInverted();
	}
}



FilterBox* TimeFilter::createFilterBox(QWidget* parent)
{
	return new TimeFilterBox(parent, name, *this);
}



QStringList TimeFilter::encodeTypeSpecific() const
{
	return {
		encodeTime("min", min),
		encodeTime("max", max)
	};
}

TimeFilter* TimeFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const QTime min = decodeTime(restOfEncoding, "min", ok);
	if (!ok) return nullptr;
	const QTime max = decodeTime(restOfEncoding, "min", ok, true);
	if (!ok) return nullptr;
	
	TimeFilter* const filter = new TimeFilter(tableToFilter, columnToFilterBy, name);
	filter->min = min;
	filter->max = max;
	
	return filter;
}
