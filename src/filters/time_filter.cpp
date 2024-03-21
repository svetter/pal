#include "time_filter.h"

#include "src/filters/filter_widgets/time_filter_box.h"



TimeFilter::TimeFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Time, tableToFilter, columnToFilterBy, name),
	min(QTime()),
	max(QTime())
{}



void TimeFilter::setMinMax(const QTime& min, const QTime& max)
{
	this->min = min;
	this->max = max;
}



unique_ptr<FilterBox> TimeFilter::getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	TimeFilter* const castPointer = (TimeFilter*) thisFilter.release();
	unique_ptr<TimeFilter> castUnique = unique_ptr<TimeFilter>(castPointer);
	
	return make_unique<TimeFilterBox>(parent, name, std::move(castUnique));
}



QStringList TimeFilter::encodeTypeSpecific() const
{
	return {
		encodeTime("min", min),
		encodeTime("max", max)
	};
}

unique_ptr<TimeFilter> TimeFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const QTime min = decodeTime(restOfEncoding, "min", ok);
	if (!ok) return nullptr;
	const QTime max = decodeTime(restOfEncoding, "min", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<TimeFilter> filter = make_unique<TimeFilter>(tableToFilter, columnToFilterBy, name);
	filter->min = min;
	filter->max = max;
	
	return filter;
}
