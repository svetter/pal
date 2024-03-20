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



FilterBox* IDFilter::getFilterBox(QWidget* parent) const
{
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
	
	return new IDFilterBox(parent, name, populateItemCombo);
}
