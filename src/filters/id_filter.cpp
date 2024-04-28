#include "id_filter.h"

#include "src/db/database.h"
#include "src/dialogs/item_dialog.h"
#include "src/filters/filter_widgets/id_filter_box.h"

#include <QComboBox>

using std::placeholders::_1, std::placeholders::_2;



IDFilter::IDFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName) :
	Filter(ID, tableToFilter, columnToFilterBy, uiName),
	value(ItemID())
{}

IDFilter::~IDFilter()
{}



bool IDFilter::evaluate(const QVariant& rawRowValue) const
{
	/*                              ╔═══════════════════════════════════════════════╗
	 *                              ║           Value from filtered table           ║
	 * ╔════════════════════════════╬───────────────┬────────────────┬──────────────╢
	 * ║        Filter              ║ QVariant null │ Invalid ItemID │ Valid ItemID ║
	 * ╟───────────┬────────────────╬═══════════════╪════════════════╪══════════════╣
	 * ║  Include  │ Invalid ItemID ║ true          │ true           │ false        ║
	 * ║    i.e.   ├────────────────╫───────────────┼────────────────┼──────────────╢
	 * ║ !inverted │   Valid ItemID ║ false         │ false          │ match        ║
	 * ╟───────────┼────────────────╫───────────────┼────────────────┼──────────────╢
	 * ║  Exclude  │ Invalid ItemID ║ false         │ false          │ true         ║
	 * ║    i.e.   ├────────────────╫───────────────┼────────────────┼──────────────╢
	 * ║  inverted │   Valid ItemID ║ true          │ true           │ !match       ║
	 * ╚═══════════╧════════════════╩═══════════════╧════════════════╧══════════════╝
	 */
	
	if (rawRowValue.isNull()) {
		return value.isValid() == isInverted();
	}
	else {
		assert(rawRowValue.canConvert<int>());
		const ItemID convertedValue = ItemID(rawRowValue.toInt());
		
		if (convertedValue.isInvalid()) {
			return value.isValid() == isInverted();
		}
		else {
			const bool match = convertedValue == value;
			return match != isInverted();
		}
	}
}



FilterBox* IDFilter::createFilterBox(QWidget* parent)
{
	assert(columnToFilterBy.contentType == ID);
	const Column* contentColumn = nullptr;
	switch (columnToFilterBy.type) {
	case Direct:	contentColumn = &((DirectCompositeColumn&)		columnToFilterBy).contentColumn;	break;
	case Reference:	contentColumn = &((ReferenceCompositeColumn&)	columnToFilterBy).contentColumn;	break;
	default: assert(false);
	}
	assert(contentColumn);
	assert(contentColumn->primaryKey);
	NormalTable* idTable = (NormalTable*) &contentColumn->foreignColumn->table;
	Database& db = idTable->db;
	
	std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo;
	     if (idTable == &db.ascentsTable)	populateItemCombo = std::bind(populateAscentCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.peaksTable)		populateItemCombo = std::bind(populatePeakCombo,	std::ref(db), _1, _2,	ItemID());
	else if (idTable == &db.tripsTable)		populateItemCombo = std::bind(populateTripCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.hikersTable)	populateItemCombo = std::bind(populateHikerCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.regionsTable)	populateItemCombo = std::bind(populateRegionCombo,	std::ref(db), _1, _2,	false);
	else if (idTable == &db.rangesTable)	populateItemCombo = std::bind(populateRangeCombo,	std::ref(db), _1, _2);
	else if (idTable == &db.countriesTable)	populateItemCombo = std::bind(populateCountryCombo,	std::ref(db), _1, _2);
	else assert(false);
	
	return new IDFilterBox(parent, uiName, populateItemCombo, *this);
}



QStringList IDFilter::encodeTypeSpecific() const
{
	return {
		encodeID("value", value)
	};
}

IDFilter* IDFilter::decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding)
{
	bool ok = false;
	
	const ItemID value = decodeID(restOfEncoding, "value", ok);
	if (!ok) return nullptr;
	
	IDFilter* const filter = new IDFilter(tableToFilter, columnToFilterBy, uiName);
	filter->value = value;
	
	return filter;
}
