#ifndef PARTICIPATED_TABLE_H
#define PARTICIPATED_TABLE_H

#include "src/db/associative_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>


class ParticipatedTable : public AssociativeTable {
public:
	Column* const ascentIDColumn;
	Column* const hikerIDColumn;
	
	ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn);
	
	void addRows(QWidget* parent, const Ascent* ascent);
	void updateRows(QWidget* parent, const Ascent* ascent);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, ValidItemID ascentID, ValidItemID hikerID) const;
};



#endif // PARTICIPATED_TABLE_H
