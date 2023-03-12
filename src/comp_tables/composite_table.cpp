#include "composite_table.h"



CompositeTable::CompositeTable(Database* db, NormalTable* baseTable) :
		QAbstractTableModel(),
		baseTable(baseTable),
		db(db),
		columns(QList<const CompositeColumn*>()),
		buffer(QList<QList<QVariant>*>()),
		bufferOrder(QList<int>()),
		currentSorting({nullptr, Qt::AscendingOrder}),
		columnsToUpdate(QSet<const CompositeColumn*>()),
		updateImmediately(false),
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

const CompositeColumn* CompositeTable::getColumnAt(int columnIndex) const
{
	return columns.at(columnIndex);
}

int CompositeTable::getIndexOf(const CompositeColumn* column) const
{
	return columns.indexOf(column);
}

const NormalTable* CompositeTable::getBaseTable() const
{
	return baseTable;
}



void CompositeTable::initBuffer(QProgressDialog* progressDialog)
{
	assert(buffer.isEmpty() && bufferOrder.isEmpty());
	
	int numberOfRows = baseTable->getNumberOfRows();
	beginResetModel();
	
	for (int bufferRowIndex = 0; bufferRowIndex < numberOfRows; bufferRowIndex++) {
		QList<QVariant>* newRow = new QList<QVariant>();
		for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
			newRow->append(computeCellContent(bufferRowIndex, columnIndex));
			
			if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
		}
		buffer.append(newRow);
		
		int viewRowIndex = findOrderIndexForInsertedItem(bufferRowIndex);
		bufferOrder.insert(viewRowIndex, bufferRowIndex);
	}
	
	endResetModel();
	columnsToUpdate.clear();
}

int CompositeTable::getNumberOfCellsToUpdate() const
{
	return columnsToUpdate.size() * buffer.size();
}

void CompositeTable::updateBuffer(QProgressDialog* progressDialog)
{
	if (columnsToUpdate.isEmpty()) return;
	
	for (const CompositeColumn* column : columnsToUpdate) {
		int columnIndex = column->getIndex();
		for (int bufferRowIndex = 0; bufferRowIndex < buffer.size(); bufferRowIndex++) {
			QVariant newContent = computeCellContent(bufferRowIndex, columnIndex);
			buffer.at(bufferRowIndex)->replace(columnIndex, newContent);
			
			if (progressDialog) progressDialog->setValue(progressDialog->value() + 1);
		}
		
		QModelIndex topLeftIndex		= index(0, columnIndex);
		QModelIndex bottomRightIndex	= index(bufferOrder.size(), columnIndex);
		Q_EMIT dataChanged(topLeftIndex, bottomRightIndex);
	}
	
	if (columnsToUpdate.contains(currentSorting.first)) {
		performSortByColumn(currentSorting.first, currentSorting.second, false);
	}
	
	columnsToUpdate.clear();
}

void CompositeTable::resetBuffer()
{
	beginResetModel();
	bufferOrder.clear();
	endResetModel();
	
	qDeleteAll(buffer);
	buffer.clear();
}

int CompositeTable::getBufferRowForViewRow(int viewRowIndex) const
{
	return bufferOrder.at(viewRowIndex);
}

int CompositeTable::findCurrentViewRowIndex(int bufferRowIndex) const
{
	for (int i = 0; i < bufferOrder.size(); i++) {
		if (bufferOrder.at(i) == bufferRowIndex) return i;
	}
	return -1;
}



QPair<const CompositeColumn*, Qt::SortOrder> CompositeTable::getCurrentSorting() const
{
	return currentSorting;
}



void CompositeTable::setUpdateImmediately(bool updateImmediately, QProgressDialog* progress)
{
	this->updateImmediately = updateImmediately;
	if (updateImmediately) updateBuffer(progress);
}

void CompositeTable::insertRowAndAnnounce(int bufferRowIndex)
{
	QList<QVariant>* newRow = new QList<QVariant>();
	for (int columnIndex = 0; columnIndex < columns.size(); columnIndex++) {
		newRow->append(computeCellContent(bufferRowIndex, columnIndex));
	}
	buffer.insert(bufferRowIndex, newRow);
	
	// Update order buffer
	for (int i = 0; i < bufferOrder.size(); i++) {
		int currentBufferRowIndex = bufferOrder.at(i);
		if (currentBufferRowIndex > bufferRowIndex) {
			bufferOrder.replace(i, currentBufferRowIndex + 1);
		}
	}
	
	int viewRowIndex = findOrderIndexForInsertedItem(bufferRowIndex);
	beginInsertRows(QModelIndex(), viewRowIndex, viewRowIndex);
	bufferOrder.insert(viewRowIndex, bufferRowIndex);
	endInsertRows();
}

void CompositeTable::removeRowAndAnnounce(int bufferRowIndex)
{
	int viewRowIndex = findCurrentViewRowIndex(bufferRowIndex);
	beginRemoveRows(QModelIndex(), viewRowIndex, viewRowIndex);
	bufferOrder.removeAt(viewRowIndex);
	endRemoveRows();
	
	// Update order buffer
	for (int i = 0; i < bufferOrder.size(); i++) {
		int currentBufferRowIndex = bufferOrder.at(i);
		if (currentBufferRowIndex > bufferRowIndex) {
			bufferOrder.replace(i, currentBufferRowIndex - 1);
		}
	}
	
	QList<QVariant>* rowToRemove = buffer.at(bufferRowIndex);
	buffer.removeAt(bufferRowIndex);
	delete rowToRemove;
}

void CompositeTable::announceChangesUnderColumn(int columnIndex)
{
	columnsToUpdate.insert(columns.at(columnIndex));
	if (updateImmediately) updateBuffer(nullptr);
}

int CompositeTable::updateSortingAfterItemEdit(int viewRowIndex)
{
	int bufferRowIndex = bufferOrder.at(viewRowIndex);
	
	beginRemoveRows(QModelIndex(), viewRowIndex, viewRowIndex);
	bufferOrder.removeAt(viewRowIndex);
	endRemoveRows();
	
	int newViewRowIndex = findOrderIndexForInsertedItem(bufferRowIndex);
	beginInsertRows(QModelIndex(), newViewRowIndex, newViewRowIndex);
	bufferOrder.insert(newViewRowIndex, bufferRowIndex);
	endInsertRows();
	
	return newViewRowIndex;
}



int CompositeTable::rowCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return bufferOrder.size();
}

int CompositeTable::columnCount(const QModelIndex& parent) const
{
	assert(!parent.isValid());
	return columns.size();
}

QVariant CompositeTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Orientation::Vertical) {
		assert(section >= 0 && section < bufferOrder.size());
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
	
	int viewRowIndex = index.row();
	assert(viewRowIndex >= 0 && viewRowIndex < bufferOrder.size());
	int bufferRowIndex = bufferOrder.at(viewRowIndex);
	assert(bufferRowIndex >= 0 && bufferRowIndex < buffer.size());
	
	int columnIndex = index.column();
	assert(columnIndex >= 0 && columnIndex < columns.size());
	const CompositeColumn* column = columns.at(columnIndex);
	
	if (role == Qt::TextAlignmentRole) {
		return column->alignment;
	}
	
	int relevantRole = column->contentType == Bit ? Qt::CheckStateRole : Qt::DisplayRole;
	if (role != relevantRole) return QVariant();
	
	QVariant result = buffer.at(bufferRowIndex)->at(columnIndex);
	
	if (result.isNull()) return QVariant();
	
	if (column->contentType == Bit) {
		assert(role == Qt::CheckStateRole);
		return result.toBool() ? Qt::Checked : Qt::Unchecked;
	}
	
	return column->toFormattedTableContent(result);
}

void CompositeTable::sort(int columnIndex, Qt::SortOrder order)
{
	assert(columnIndex >= 0 && columnIndex < columns.size());
	const CompositeColumn* const column = columns.at(columnIndex);
	
	performSortByColumn(column, order, true);
}

void CompositeTable::performSortByColumn(const CompositeColumn* column, Qt::SortOrder order, bool allowPassAndReverse)
{
	assert(column);
	
	if (allowPassAndReverse && column == currentSorting.first) {
		if (order == currentSorting.second) return;
		
		std::reverse(bufferOrder.begin(), bufferOrder.end());
	}
	else {
		auto comparator = [&column, order](int i1, int i2) {
			QVariant value1 = column->getValueAt(i1);
			QVariant value2 = column->getValueAt(i2);
			
			if (order == Qt::AscendingOrder) {
				return column->compare(value1, value2);
			} else {
				return column->compare(value2, value1);
			}
		};
		
		std::stable_sort(bufferOrder.begin(), bufferOrder.end(), comparator);
	}
	
	// Notify model users (views)
	QModelIndex topLeftIndex		= index(0, 0);
	QModelIndex bottomRightIndex	= index(bufferOrder.size() - 1, columns.size() - 1);
	Q_EMIT dataChanged(topLeftIndex, bottomRightIndex);
	//headerDataChanged(Qt::Vertical, 0, bufferOrder.size() - 1);
	
	currentSorting = {column, order};
}



QVariant CompositeTable::computeCellContent(int bufferRowIndex, int columnIndex) const
{
	assert(bufferRowIndex >= 0 && bufferRowIndex < baseTable->getNumberOfRows());
	assert(columnIndex >= 0 && columnIndex < columns.size());
	
	const CompositeColumn* column = columns.at(columnIndex);
	QVariant result = column->getValueAt(bufferRowIndex);
	
	if (!result.isValid()) return QVariant();
	
	return result;
}



int CompositeTable::findOrderIndexForInsertedItem(int insertedItemBufferRowIndex)
{
	if (!currentSorting.first) {
		qDebug() << "Warning: Inserting into unsorted composite table" << name;
		return bufferOrder.size();
	}
	
	const CompositeColumn* currentSortColumn = currentSorting.first;
	const QVariant ownCompareValue = currentSortColumn->getValueAt(insertedItemBufferRowIndex);
	for (int i = 0; i < bufferOrder.size(); i++) {
		const QVariant compareTo = currentSortColumn->getValueAt(bufferOrder.at(i));
		bool insertHere;
		if (currentSorting.second == Qt::AscendingOrder) {
			insertHere = currentSortColumn->compare(ownCompareValue, /* < */ compareTo);
		} else {
			insertHere = currentSortColumn->compare(compareTo, /* < */ ownCompareValue);
		}
		
		if (insertHere) {
			return i;
		}
	}
	return bufferOrder.size();
}
