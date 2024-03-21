#include "bool_filter.h"

#include "src/filters/filter_widgets/bool_filter_box.h"



BoolFilter::BoolFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(Bit, tableToFilter, columnToFilterBy, name),
	value(true)
{}



void BoolFilter::setValue(bool value)
{
	this->value = value;
}



unique_ptr<FilterBox> BoolFilter::getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	BoolFilter* const castPointer = (BoolFilter*) thisFilter.release();
	unique_ptr<BoolFilter> castUnique = unique_ptr<BoolFilter>(castPointer);
	
	return make_unique<BoolFilterBox>(parent, name, std::move(castUnique));
}



QStringList BoolFilter::encodeTypeSpecific() const
{
	return {
		encodeBool("value", value)
	};
}

unique_ptr<BoolFilter> BoolFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const bool value = decodeBool(restOfEncoding, "value", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<BoolFilter> filter = make_unique<BoolFilter>(tableToFilter, columnToFilterBy, name);
	filter->value = value;
	
	return filter;
}
