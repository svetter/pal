#include "composite_column.h"

#include "src/db/normal_table.h"
#include "src/db/table.h"



CompositeColumn::CompositeColumn(QString uiName, Qt::AlignmentFlag alignment) :
		uiName(uiName),
		alignment(alignment)
{}





DirectCompositeColumn::DirectCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, const Column* baseColumn) :
		CompositeColumn(uiName, alignment),
		baseColumn(baseColumn)
{}



QVariant DirectCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	return baseColumn->table->getBufferRow(rowIndex)->at(baseColumn->getIndex());
}





ReferenceCompositeColumn::ReferenceCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, QList<const Column*> foreignKeyColumnSequence, const Column* contentColumn) :
		CompositeColumn(uiName, alignment),
		foreignKeyColumnSequence(foreignKeyColumnSequence),
		contentColumn(contentColumn)
{}



QVariant ReferenceCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	assert(foreignKeyColumnSequence.first()->isForeignKey());
	
	int currentRowIndex = rowIndex;
	assert(!foreignKeyColumnSequence.first()->table->isAssociative);
	NormalTable* currentTable = (NormalTable*) foreignKeyColumnSequence.first()->table;
	
	for (const Column* currentColumn : foreignKeyColumnSequence) {
		qDebug() << currentTable;
		qDebug() << currentTable->name;
		qDebug() << currentColumn->table->name;
		assert(currentColumn->table == currentTable);
		assert(currentColumn->isForeignKey());
		
		// Look up key stored in current column at current row index
		ItemID key = currentTable->getBufferRow(currentRowIndex)->at(currentColumn->getIndex()).toInt();
		
		if (key.isNull()) return QVariant();
		
		// Get referenced primary key column of other table
		const Column* referencedColumn = currentColumn->getReferencedForeignColumn();
		assert(!referencedColumn->table->isAssociative);
		currentTable = (NormalTable*) referencedColumn->table;
		
		// Find row index that contains the current primary key
		currentRowIndex = currentTable->getBufferIndexForPrimaryKey(key.forceValid());
		assert(currentRowIndex >= 0);
	}
	
	// Finally, look up content column at last row index
	assert(contentColumn->table == currentTable);
	return currentTable->getBufferRow(currentRowIndex)->at(contentColumn->getIndex());
}





FoldCompositeColumn::FoldCompositeColumn(QString uiName, CompositeColumnFoldOp op, const QList<QPair<const Column*, const Column*>> breadcrumbs, const Column* contentColumn) :
		CompositeColumn(uiName, op == List ? Qt::AlignLeft : Qt::AlignRight),
		op(op),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn)
{
	assert((op == Count) == (contentColumn == nullptr));
}



QVariant FoldCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	QSet<int> currentRowIndexSet = { rowIndex };
	const Table* currentTable = (NormalTable*) breadcrumbs.first().first->table;
	
	for (int round = 0; round < breadcrumbs.size(); round++) {
		const Column* firstColumn	= breadcrumbs.at(round).first;
		const Column* secondColumn	= breadcrumbs.at(round).second;
		
		// Check continuity
		assert(firstColumn->table == currentTable);
		assert(firstColumn->table != secondColumn->table);
		assert(firstColumn->isForeignKey() != secondColumn->isForeignKey());
		
		// Look up keys stored in firstColumn at given row indices
		QSet<ValidItemID> currentKeySet = QSet<ValidItemID>();
		for (int rowIndex : currentRowIndexSet) {
			ValidItemID key = currentTable->getBufferRow(rowIndex)->at(firstColumn->getIndex()).toInt();
			currentKeySet.insert(key);
		}
		
		currentRowIndexSet.clear();
		currentTable = secondColumn->table;
		
		// The second half of the transfer is dependent on the reference direction:
		if (firstColumn->isForeignKey()) {
			// Forward reference (lookup, result for each input element is single key)
			assert(firstColumn->getReferencedForeignColumn() == secondColumn);
			assert(secondColumn->isPrimaryKey());
			
			// Find row matching each primary key
			for (ValidItemID key : currentKeySet) {
				int rowIndex = currentTable->getMatchingBufferRowIndex({ secondColumn }, { key });
				currentRowIndexSet.insert(rowIndex);
			}
		}
		else if (secondColumn->isForeignKey()) {
			// Backward reference (reference search, result for each input element is key set)
			assert(firstColumn == secondColumn->getReferencedForeignColumn());
			assert(firstColumn->isPrimaryKey());
			
			// Find rows in new currentTable where key in secondColumn matches any key in current set
			for (ValidItemID key : currentKeySet) {
				const QList<int> indexList = currentTable->getMatchingBufferRowIndices(secondColumn, key.asQVariant());
				const QSet<int> matchingBufferRowIndices = QSet<int>(indexList.constBegin(), indexList.constEnd());
				currentRowIndexSet.unite(matchingBufferRowIndices);
			}
		}
		else assert(false);
		
		if (currentRowIndexSet.isEmpty()) {
			if (op == Count) return 0;
			return QVariant();
		}
	}
	
	// RETURN IF COUNT
	if (op == Count) return currentRowIndexSet.size();
	
	
	
	// EXECUTE FOLD OPERATION
	
	assert(currentTable == contentColumn->table);
	assert(!currentTable->isAssociative);
	NormalTable* contentTable = (NormalTable*) currentTable;
	
	int aggregate = 0;
	QString listString = "";
	
	for (int rowIndex : currentRowIndexSet) {
		QVariant contentValue = contentTable->getBufferRow(rowIndex)->at(contentColumn->getIndex());
		
		switch (op) {
		case List:
			assert(contentValue.canConvert<QString>());
			if (!listString.isEmpty()) listString.append(", ");
			listString.append(contentValue.toString());
			break;
		case Average:
		case Sum:
			assert(contentValue.canConvert<int>());
			aggregate += contentValue.toInt();
			break;
		case Max:
			assert(contentValue.canConvert<int>());
			if (contentValue.toInt() > aggregate) aggregate = contentValue.toInt();
			break;
		default:
			assert(false);
		}
	}
	
	assert(currentRowIndexSet.size() > 0);
	
	switch (op) {
	case List:		return listString;
	case Average:	return aggregate / currentRowIndexSet.size();
	case Sum:		return aggregate;
	case Max:		return aggregate;
	default:		assert(false);
	}
	
	return QVariant();
}
