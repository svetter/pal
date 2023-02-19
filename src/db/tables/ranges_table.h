#ifndef RANGES_TABLE_H
#define RANGES_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/range.h"

#include <QWidget>



class RangesTable : public NormalTable {
	Q_OBJECT
	
public:
	const Column* nameColumn;
	const Column* continentColumn;
	
	RangesTable();
	
	int addRow(QWidget* parent, const Range* range);
	void updateRow(QWidget* parent, ValidItemID rangeID, const Range* range);
	
	QList<QVariant> mapDataToQVariantList(const Range* range) const;
	
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // RANGES_TABLE_H
