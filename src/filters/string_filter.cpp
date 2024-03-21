#include "string_filter.h"

#include "src/filters/filter_widgets/string_filter_box.h"



StringFilter::StringFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(String, tableToFilter, columnToFilterBy, name),
	value(QString())
{}



void StringFilter::setValue(const QString& value)
{
	this->value = value;
}



unique_ptr<FilterBox> StringFilter::getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	StringFilter* const castPointer = (StringFilter*) thisFilter.release();
	unique_ptr<StringFilter> castUnique = unique_ptr<StringFilter>(castPointer);
	
	return make_unique<StringFilterBox>(parent, name, std::move(castUnique));
}



QStringList StringFilter::encodeTypeSpecific() const
{
	return {
		encodeString("value", value)
	};
}

unique_ptr<StringFilter> StringFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const QString value = decodeString(restOfEncoding, "value", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<StringFilter> filter = make_unique<StringFilter>(tableToFilter, columnToFilterBy, name);
	filter->value = value;
	
	return filter;
}
