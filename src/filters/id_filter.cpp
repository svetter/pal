#include "id_filter.h"

#include "src/db/database.h"
#include "src/dialogs/item_dialog.h"
#include "src/filters/filter_widgets/id_filter_box.h"

#include <QComboBox>



IDFilter::IDFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	Filter(ID, tableToFilter, columnToFilterBy, name),
	value(ItemID())
{}



void IDFilter::setValue(const ItemID& value)
{
	this->value = value;
}



unique_ptr<FilterBox> IDFilter::getFilterBox(QWidget* parent, unique_ptr<Filter> thisFilter) const
{
	IDFilter* const castPointer = (IDFilter*) thisFilter.release();
	unique_ptr<IDFilter> castUnique = unique_ptr<IDFilter>(castPointer);
	
	Database& db = tableToFilter.db;
	assert(columnToFilterBy.isForeignKey());
	NormalTable* idTable = (NormalTable*) &columnToFilterBy.getReferencedForeignColumn().table;
	
	std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo;
	     if (idTable == &db.ascentsTable)	populateItemCombo = std::bind(populateAscentCombo,	std::ref(db), std::placeholders::_1, std::placeholders::_2);
	else if (idTable == &db.peaksTable)		populateItemCombo = std::bind(populatePeakCombo,	std::ref(db), std::placeholders::_1, std::placeholders::_2,	ItemID());
	else if (idTable == &db.tripsTable)		populateItemCombo = std::bind(populateTripCombo,	std::ref(db), std::placeholders::_1, std::placeholders::_2);
	else if (idTable == &db.hikersTable)	populateItemCombo = std::bind(populateHikerCombo,	std::ref(db), std::placeholders::_1, std::placeholders::_2);
	else if (idTable == &db.regionsTable)	populateItemCombo = std::bind(populateRegionCombo,	std::ref(db), std::placeholders::_1, std::placeholders::_2,	false);
	else if (idTable == &db.rangesTable)	populateItemCombo = std::bind(populateRangeCombo,	std::ref(db), std::placeholders::_1, std::placeholders::_2);
	else if (idTable == &db.countriesTable)	populateItemCombo = std::bind(populateCountryCombo,	std::ref(db), std::placeholders::_1, std::placeholders::_2);
	else assert(false);
	
	return make_unique<IDFilterBox>(parent, name, populateItemCombo, std::move(castUnique));
}



QStringList IDFilter::encodeTypeSpecific() const
{
	return {
		encodeID("value", value)
	};
}

unique_ptr<IDFilter> IDFilter::decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding)
{
	bool ok = false;
	
	const ItemID value = decodeID(restOfEncoding, "value", ok, true);
	if (!ok) return nullptr;
	
	unique_ptr<IDFilter> filter = make_unique<IDFilter>(tableToFilter, columnToFilterBy, name);
	filter->value = value;
	
	return filter;
}
