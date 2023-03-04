#include "composite_table.h"



CompositeTable::CompositeTable(Database* db, NormalTable* baseTable) :
		QAbstractTableModel(),
		baseTable(baseTable),
		db(db)
{}



void CompositeTable::addColumn(const CompositeColumn* column)
{
	columns.append(column);
}



int CompositeTable::rowCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return baseTable->getNumberOfRows();
}

int CompositeTable::columnCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return 8;
}

QVariant CompositeTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Orientation::Vertical) {
		switch (role) {
		case Qt::TextAlignmentRole:	return Qt::AlignRight;
		case Qt::DisplayRole:		return section + 1;
		}
		return QVariant();
	}
	
	if (role != Qt::DisplayRole) return QVariant();
	
	assert(section >= 0 && section < columns.size());
	return columns.at(section)->uiName;
}

QVariant CompositeTable::data(const QModelIndex& index, int role) const
{
	assert(!index.parent().isValid());
	
	if (index.column() < 8) return columns.at(index.column())->data(index.row(), role);
	
	switch (index.column()) {
	case 7: {
		if (role != Qt::DisplayRole) break;
		ValidItemID regionID = db->regionsTable->getPrimaryKeyAt(index.row());
		int result = 0;
		for (int i = 0; i < db->ascentsTable->getNumberOfRows(); i++) {
			ItemID peakID = db->ascentsTable->getBufferRow(i)->at(db->ascentsTable->peakIDColumn->getIndex()).toInt();
			if (!peakID.isValid()) continue;
			int peakBufferRowIndex = db->peaksTable->getBufferIndexForPrimaryKey(peakID.forceValid());
			result += regionID == db->peaksTable->getBufferRow(peakBufferRowIndex)->at(db->peaksTable->regionIDColumn->getIndex()).toInt();
		}
		return result;
	}
	default:
		assert(false);
	}
	
	return QVariant();
}
