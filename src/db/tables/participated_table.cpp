#include "participated_table.h"

#include <QTranslator>



ParticipatedTable::ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn) :
		AssociativeTable(QString("Participated"), foreignAscentIDColumn, foreignHikerIDColumn),
		ascentIDColumn(getColumn1()),
		hikerIDColumn(getColumn2())
{}



void ParticipatedTable::addRows(QWidget* parent, const Ascent* ascent)
{
	for (ValidItemID hikerID : ascent->hikerIDs) {
		QList<const Column*> columns = getColumnList();
		QList<QVariant> data = mapDataToQVariantList(columns, ascent->ascentID.forceValid(), hikerID);
		
		AssociativeTable::addRow(parent, columns, data);
	}
}

void ParticipatedTable::updateRows(QWidget* parent, const Ascent* ascent)
{
	// delete pre-existing rows
	removeMatchingRows(parent, ascentIDColumn, ascent->ascentID.forceValid());
	// add back all current rows
	addRows(parent, ascent);
}


QList<QVariant> ParticipatedTable::mapDataToQVariantList(QList<const Column*>& columns, ValidItemID ascentID, ValidItemID hikerID) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == ascentIDColumn)	{ data.append(ascentID.asQVariant());	continue; }
		if (column == hikerIDColumn)	{ data.append(hikerID.asQVariant());	continue; }
		assert(false);
	}
	return data;
}
