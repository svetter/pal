#include "composite_table.h"



CompositeTable::CompositeTable(Database* db, NormalTable* baseTable) :
		QAbstractTableModel(),
		baseTable(baseTable),
		db(db),
		name(baseTable->name)
{
	baseTable->setRowChangeListener(this);
}



void CompositeTable::addColumn(const CompositeColumn* column)
{
	columns.append(column);
	
	// Register as change listener at all underlying columns
	const QSet<Column* const> underlyingColumns = column->getAllUnderlyingColumns();
	for (Column* underlyingColumn : underlyingColumns) {
		underlyingColumn->registerChangeListener(column);
	}
}

int CompositeTable::getIndexOf(const CompositeColumn* column) const
{
	return columns.indexOf(column);
}



void CompositeTable::beginInsertRow(int bufferRowIndex)
{
	beginInsertRows(QModelIndex(), bufferRowIndex, bufferRowIndex);
}

void CompositeTable::endInsertRow()
{
	endInsertRows();
}

void CompositeTable::beginRemoveRow(int bufferRowIndex)
{
	beginRemoveRows(QModelIndex(), bufferRowIndex, bufferRowIndex);
}

void CompositeTable::endRemoveRow()
{
	endRemoveRows();
}



int CompositeTable::rowCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return baseTable->getNumberOfRows();
}

int CompositeTable::columnCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return columns.size();
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
	int columnIndex = index.column();
	assert(columnIndex >= 0 && columnIndex < columns.size());
	
	return columns.at(columnIndex)->data(index.row(), role);
}
