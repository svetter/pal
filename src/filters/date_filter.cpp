#include "date_filter.h"

#include "src/filters/filter_widgets/date_filter_box.h"



DateFilter::DateFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Date, tableToFilter, columnToFilterBy, FilterFoldOp(-1), name),
	min(QDate()),
	max(QDate())
{}



void DateFilter::setMinMax(const QDate& min, const QDate& max)
{
	this->min = min;
	this->max = max;
}



bool DateFilter::evaluate(const QVariant& rawRowValue) const
{
	assert(!min.isNull());
	assert(!max.isNull());
	
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<QDate>());
		const QDate convertedValue = rawRowValue.toDate();
		const bool match = convertedValue >= min && convertedValue <= max;
		
		return match != isInverted();
	}
}



unique_ptr<FilterBox> DateFilter::createFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	DateFilter* const castPointer = (DateFilter*) thisFilter.release();
	unique_ptr<DateFilter> castUnique = unique_ptr<DateFilter>(castPointer);
	
	return make_unique<DateFilterBox>(parent, name, std::move(castUnique));
}



QStringList DateFilter::encodeTypeSpecific() const
{
	return {
		encodeDate("min", min),
		encodeDate("max", max)
	};
}

unique_ptr<DateFilter> DateFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const QDate min = decodeDate(restOfEncoding, "min", ok);
	if (!ok) return nullptr;
	const QDate max = decodeDate(restOfEncoding, "min", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<DateFilter> filter = make_unique<DateFilter>(tableToFilter, columnToFilterBy, name);
	filter->min = min;
	filter->max = max;
	
	return filter;
}
