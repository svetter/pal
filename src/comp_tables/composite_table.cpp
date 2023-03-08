#include "composite_table.h"



CompositeTable::CompositeTable(Database* db, NormalTable* baseTable) :
		QAbstractTableModel(),
		baseTable(baseTable),
		db(db),
		columns(QList<const CompositeColumn*>()),
		buffer(QList<QList<QVariant>*>()),
		rowBeingInserted(-1),
		name(baseTable->name)
{
	baseTable->setRowChangeListener(this);
}

CompositeTable::~CompositeTable()
{
	qDeleteAll(columns);
	qDeleteAll(buffer);
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



void CompositeTable::initBuffer()
{
	assert(buffer.isEmpty());
	
	int numberOfRows = baseTable->getNumberOfRows();
	beginInsertRows(QModelIndex(), 0, numberOfRows - 1);
	
	for (int rowIndex = 0; rowIndex < numberOfRows; rowIndex++) {
		QList<QVariant>* newRow = new QList<QVariant>();
		for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
			newRow->append(computeCellContent(rowIndex, columnIndex));
		}
		buffer.append(newRow);
	}
	
	endInsertRows();
}

void CompositeTable::resetBuffer()
{
	beginRemoveRows(QModelIndex(), 0, buffer.size() - 1);
	qDeleteAll(buffer);
	buffer.clear();
	endRemoveRows();
}



void CompositeTable::beginInsertRow(int bufferRowIndex)
{
	beginInsertRows(QModelIndex(), bufferRowIndex, bufferRowIndex);
	
	rowBeingInserted = bufferRowIndex;
}

void CompositeTable::endInsertRow()
{
	assert(rowBeingInserted >= 0);
	QList<QVariant>* newRow = new QList<QVariant>();
	for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
		newRow->append(computeCellContent(rowBeingInserted, columnIndex));
	}
	buffer.insert(rowBeingInserted, newRow);
	rowBeingInserted = -1;
	
	endInsertRows();
}

void CompositeTable::beginRemoveRow(int bufferRowIndex)
{
	beginRemoveRows(QModelIndex(), bufferRowIndex, bufferRowIndex);
	
	QList<QVariant>* rowToRemove = buffer.at(bufferRowIndex);
	buffer.removeAt(bufferRowIndex);
	delete rowToRemove;
}

void CompositeTable::endRemoveRow()
{
	endRemoveRows();
}



int CompositeTable::rowCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return buffer.size();
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
	int rowIndex = index.row();
	assert(rowIndex >= 0 && rowIndex < buffer.size());
	int columnIndex = index.column();
	assert(columnIndex >= 0 && columnIndex < columns.size());
	const CompositeColumn* column = columns.at(columnIndex);
	
	if (role == Qt::TextAlignmentRole) {
		return column->alignment;
	}
	
	int relevantRole = column->contentType == bit ? Qt::CheckStateRole : Qt::DisplayRole;
	if (role != relevantRole) return QVariant();
	
	QVariant result = buffer.at(rowIndex)->at(columnIndex);
	
	if (result.isNull()) return QVariant();
	
	if (column->contentType == bit) {
		assert(role == Qt::CheckStateRole);
		return result.toBool() ? Qt::Checked : Qt::Unchecked;
	}
	
	return result;
}

QVariant CompositeTable::computeCellContent(int rowIndex, int columnIndex) const
{
	assert(rowIndex >= 0 && rowIndex < baseTable->getNumberOfRows());
	assert(columnIndex >= 0 && columnIndex < columns.size());
	
	const CompositeColumn* column = columns.at(columnIndex);
	QVariant result = column->getValueAt(rowIndex);
	
	if (result.isNull()) return QVariant();
	
	if (column->contentType == date) {
		assert(result.canConvert<QDate>());
		return result.toDate().toString("dd.MM.yyyy");
	}
	
	if (column->contentType == time_) {
		assert(result.canConvert<QTime>());
		return result.toTime().toString("HH:mm");
	}
	
	return result;
}
