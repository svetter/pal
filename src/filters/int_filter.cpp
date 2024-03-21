#include "int_filter.h"

#include "src/filters/filter_widgets/int_filter_box.h"
#include "src/filters/filter_widgets/int_class_filter_box.h"



IntFilter::IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Integer, tableToFilter, columnToFilterBy, name),
	useClasses(false),
	classIncrement(-1),
	classesMinValue(-1),
	classesMaxValue(-1),
	min(0),
	max(0)
{}

IntFilter::IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, int classIncrement, int classesMinValue, int classesMaxValue) :
	Filter(Integer, tableToFilter, columnToFilterBy, name),
	useClasses(true),
	classIncrement(classIncrement),
	classesMinValue(classesMinValue),
	classesMaxValue(classesMaxValue),
	min(0),
	max(0)
{}



void IntFilter::setMinMax(int min, int max)
{
	this->min	= min;
	this->max	= max;
}



unique_ptr<FilterBox> IntFilter::getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	IntFilter* const castPointer = (IntFilter*) thisFilter.release();
	unique_ptr<IntFilter> castUnique = unique_ptr<IntFilter>(castPointer);
	
	if (useClasses) {
		return make_unique<IntClassFilterBox>(parent, name, classIncrement, classesMinValue, classesMaxValue, std::move(castUnique));
	} else {
		return make_unique<IntFilterBox>(parent, name, std::move(castUnique));
	}
}



QStringList IntFilter::encodeTypeSpecific() const
{
	return {
		encodeBool("useClasses",		useClasses),
		encodeInt("classIncrement",		classIncrement),
		encodeInt("classesMinValue",	classesMinValue),
		encodeInt("classesMaxValue",	classesMaxValue),
		encodeInt("min",				min),
		encodeInt("max",				max)
	};
}

unique_ptr<IntFilter> IntFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const bool useClasses = decodeBool(restOfEncoding, "useClasses", ok);
	if (!ok) return nullptr;
	const int classIncrement = decodeInt(restOfEncoding, "classIncrement", ok);
	if (!ok && useClasses) return nullptr;
	const int classesMinValue = decodeInt(restOfEncoding, "classesMinValue", ok);
	if (!ok && useClasses) return nullptr;
	const int classesMaxValue = decodeInt(restOfEncoding, "classesMaxValue", ok);
	if (!ok && useClasses) return nullptr;
	const int min = decodeInt(restOfEncoding, "min", ok);
	if (!ok) return nullptr;
	const int max = decodeInt(restOfEncoding, "min", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<IntFilter> filter = nullptr;
	if (useClasses) {
		filter = make_unique<IntFilter>(tableToFilter, columnToFilterBy, name, classIncrement, classesMinValue, classesMaxValue);
	} else {
		filter = make_unique<IntFilter>(tableToFilter, columnToFilterBy, name);
	}
	filter->min = min;
	filter->max = max;
	
	return filter;
}
