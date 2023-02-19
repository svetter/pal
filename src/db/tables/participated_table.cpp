#include "participated_table.h"

#include <QTranslator>



ParticipatedTable::ParticipatedTable(const Column* foreignAscentIDColumn, const Column* foreignHikerIDColumn) :
		AssociativeTable(QString("Participated"), foreignAscentIDColumn, foreignHikerIDColumn),
		ascentIDColumn(getColumn1()),
		hikerIDColumn(getColumn2())
{}



void ParticipatedTable::addRows(QWidget* parent, const Ascent* ascent)
{
	for (ValidItemID hikerID : ascent->hikerIDs) {
		addRow(parent, ascent->ascentID.get(), hikerID.get());
	}
}

void ParticipatedTable::addRow(QWidget* parent, ValidItemID ascentID, ValidItemID hikerID)
{
	QList<const Column*> columns = getColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == ascentIDColumn)	{ data.append(ascentID.get());	continue; }
		if (*iter == hikerIDColumn)		{ data.append(hikerID.get());	continue; }
		assert(false);
	}
	AssociativeTable::addRow(parent, data);
}

void ParticipatedTable::removeRow(QWidget* parent, QPair<int, int>& primaryKey)
{
	// TODO #71
	qDebug() << "UNIMPLEMENTED: ParticipatedTable::removeRow()";
}
