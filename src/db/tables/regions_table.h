#ifndef REGIONS_TABLE_H
#define REGIONS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/region.h"



class RegionsTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* const nameColumn;
	Column* const rangeIDColumn;
	Column* const countryIDColumn;
	
	RegionsTable(Column* foreignRangeIDColumn, Column* foreignCountryIDColumn);
	
	int addRow(QWidget* parent, Region* region);
	void updateRow(QWidget* parent, ValidItemID regionID, const Region* region);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Region* region) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // REGIONS_TABLE_H
