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





ReferenceCompositeColumn::ReferenceCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, const Column* keyColumn, const Column* referencedColumn) :
		CompositeColumn(uiName, alignment),
		keyColumn(keyColumn),
		referencedColumn(referencedColumn)
{}



QVariant ReferenceCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	ItemID ownID = keyColumn->table->getBufferRow(rowIndex)->at(keyColumn->getIndex()).toInt();
	if (!ownID.isValid()) return QVariant();
	assert(!referencedColumn->table->isAssociative);
	NormalTable* referencedTable = (NormalTable*) referencedColumn->table;
	
	int referencedItemBufferRowIndex = referencedTable->getBufferIndexForPrimaryKey(ownID.forceValid());
	return referencedTable->getBufferRow(referencedItemBufferRowIndex)->at(referencedColumn->getIndex());
}





CountCompositeColumn::CountCompositeColumn(QString uiName, QList<QPair<const Column*, const Column*>> breadcrumbs) :
		CompositeColumn(uiName, Qt::AlignRight),
		breadcrumbs(breadcrumbs)
{}



QVariant CountCompositeColumn::data(int rowIndex, int role) const
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
		
		if (currentRowIndexSet.isEmpty()) return 0;
	}
	
	return currentRowIndexSet.size();
}




FoldCompositeColumn::FoldCompositeColumn(QString uiName, CompositeColumnFoldOp op, const QList<QPair<const Column*, const Column*>> breadcrumbs, const Column* contentColumn) :
		CompositeColumn(uiName, Qt::AlignRight),
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





SimpleFoldCompositeColumn::SimpleFoldCompositeColumn(QString uiName, CompositeColumnFoldOp op, const Column* keyColumn, const Column* matchingColumn, const Column* contentColumn) :
		CompositeColumn(uiName, Qt::AlignRight),
		op(op),
		keyColumn(keyColumn),
		matchingColumn(matchingColumn),
		contentColumn(contentColumn)
{
	assert(matchingColumn->table == contentColumn->table);
}



QVariant SimpleFoldCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	assert(!keyColumn->table->isAssociative);
	assert(!matchingColumn->table->isAssociative);
	NormalTable* baseTable = (NormalTable*) keyColumn->table;
	NormalTable* otherTable = (NormalTable*) matchingColumn->table;
	
	ValidItemID ownID = baseTable->getPrimaryKeyAt(rowIndex);
	int numberOfElements = 0;
	int aggregate = 0;
	QString listString = "";
	
	for (int i = 0; i < otherTable->getNumberOfRows(); i++) {
		const QList<QVariant>* bufferRow = otherTable->getBufferRow(i);
		if (ownID == bufferRow->at(matchingColumn->getIndex()).toInt()) {
			numberOfElements++;
			QVariant contentValue = bufferRow->at(contentColumn->getIndex());
			
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
	}
	
	if (numberOfElements == 0) return QVariant();
	
	switch (op) {
	case List:		return listString;
	case Average:	return aggregate / numberOfElements;
	case Sum:		return aggregate;
	case Max:		return aggregate;
	default:		assert(false);
	}
	
	return QVariant();
}
