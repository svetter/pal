#include "int_filter.h"

#include "src/filters/filter_widgets/int_filter_box.h"
#include "src/filters/filter_widgets/int_class_filter_box.h"



IntFilter::IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, NumericFoldOp foldOp, const QString& name) :
	Filter(Integer, tableToFilter, columnToFilterBy, foldOp, name),
	useClasses(false),
	classIncrement(-1),
	classesMinValue(-1),
	classesMaxValue(-1),
	min(0),
	max(1)
{}

IntFilter::IntFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, NumericFoldOp foldOp, const QString& name, int classIncrement, int classesMinValue, int classesMaxValue) :
	Filter(Integer, tableToFilter, columnToFilterBy, foldOp, name),
	useClasses(true),
	classIncrement(classIncrement),
	classesMinValue(classesMinValue),
	classesMaxValue(classesMaxValue),
	min(0),
	max(0)
{}

IntFilter::~IntFilter()
{}



bool IntFilter::evaluate(const QVariant& rawRowValue) const
{
	if (rawRowValue.isNull()) {
		return isInverted();
	}
	else {
		assert(rawRowValue.canConvert<int>());
		const int convertedValue = rawRowValue.toInt();
		const bool match = convertedValue >= min && convertedValue <= max;
		
		return match != isInverted();
	}
}



FilterBox* IntFilter::createFilterBox(QWidget* parent)
{
	if (useClasses) {
		return new IntClassFilterBox(parent, name, classIncrement, classesMinValue, classesMaxValue, *this);
	} else {
		return new IntFilterBox(parent, name, *this);
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

IntFilter* IntFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, NumericFoldOp foldOp, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const bool useClasses = decodeBool(restOfEncoding, "useClasses", ok);
	if (!ok)
		return nullptr;
	const int classIncrement = decodeInt(restOfEncoding, "classIncrement", ok);
	if (!ok && useClasses)
		return nullptr;
	const int classesMinValue = decodeInt(restOfEncoding, "classesMinValue", ok);
	if (!ok && useClasses)
		return nullptr;
	const int classesMaxValue = decodeInt(restOfEncoding, "classesMaxValue", ok);
	if (!ok && useClasses)
		return nullptr;
	const int min = decodeInt(restOfEncoding, "min", ok);
	if (!ok)
		return nullptr;
	const int max = decodeInt(restOfEncoding, "max", ok);
	if (!ok)
		return nullptr;
	
	IntFilter* filter = nullptr;
	if (useClasses) {
		filter = new IntFilter(tableToFilter, columnToFilterBy, foldOp, name, classIncrement, classesMinValue, classesMaxValue);
	} else {
		filter = new IntFilter(tableToFilter, columnToFilterBy, foldOp, name);
	}
	filter->min = min;
	filter->max = max;
	
	return filter;
}
