#include "date_filter.h"

#include "src/filters/filter_widgets/date_filter_box.h"



DateFilter::DateFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Date, tableToFilter, columnToFilterBy, NumericFoldOp(-1), name),
	min(QDate::currentDate()),
	max(QDate::currentDate())
{}

DateFilter::~DateFilter()
{}



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



FilterBox* DateFilter::createFilterBox(QWidget* parent)
{
	return new DateFilterBox(parent, name, *this);
}



QStringList DateFilter::encodeTypeSpecific() const
{
	return {
		encodeDate("min", min),
		encodeDate("max", max)
	};
}

DateFilter* DateFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const QDate min = decodeDate(restOfEncoding, "min", ok);
	if (!ok) return nullptr;
	const QDate max = decodeDate(restOfEncoding, "max", ok);
	if (!ok) return nullptr;
	
	DateFilter* const filter = new DateFilter(tableToFilter, columnToFilterBy, name);
	filter->min = min;
	filter->max = max;
	
	return filter;
}
