#include "composite_column.h"

#include "qdatetime.h"
#include "src/db/normal_table.h"
#include "src/db/table.h"



CompositeColumn::CompositeColumn(QString uiName, Qt::AlignmentFlag alignment) :
		uiName(uiName),
		alignment(alignment)
{}



QVariant CompositeColumn::replaceEnumIfApplicable(QVariant content, const QStringList* enumNames)
{
	if (!enumNames) return content;
	
	// If content is enumerative, replace index with the corresponding string
	assert(content.canConvert<int>());
	int index = content.toInt();
	assert(index >= 0 && index < enumNames->size());
	return enumNames->at(index);
}





DirectCompositeColumn::DirectCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, const Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(uiName, alignment),
		contentColumn(contentColumn),
		enumNames(enumNames)
{}



QVariant DirectCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	QVariant content = contentColumn->table->getBufferRow(rowIndex)->at(contentColumn->getIndex());
	content = replaceEnumIfApplicable(content, enumNames);
	return content;
}





ReferenceCompositeColumn::ReferenceCompositeColumn(QString uiName, Qt::AlignmentFlag alignment, QList<const Column*> foreignKeyColumnSequence, const Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(uiName, alignment),
		foreignKeyColumnSequence(foreignKeyColumnSequence),
		contentColumn(contentColumn),
		enumNames(enumNames)
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
	QVariant content = currentTable->getBufferRow(currentRowIndex)->at(contentColumn->getIndex());
	content = replaceEnumIfApplicable(content, enumNames);
	
	return content;
}





FoldCompositeColumn::FoldCompositeColumn(QString uiName, FoldOp op, const QList<QPair<const Column*, const Column*>> breadcrumbs, const Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(uiName, op == List ? Qt::AlignLeft : Qt::AlignRight),
		op(op),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn),
		enumNames(enumNames)
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
		QVariant content = contentTable->getBufferRow(rowIndex)->at(contentColumn->getIndex());
		
		switch (op) {
		case List:
			assert(content.canConvert<QString>());
			if (!listString.isEmpty()) listString.append(", ");
			content = replaceEnumIfApplicable(content, enumNames);
			listString.append(content.toString());
			break;
		case Average:
		case Sum:
			assert(content.canConvert<int>());
			aggregate += content.toInt();
			break;
		case Max:
			assert(content.canConvert<int>());
			if (content.toInt() > aggregate) aggregate = content.toInt();
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





DifferenceCompositeColumn::DifferenceCompositeColumn(QString uiName, const Column* minuendColumn, const Column* subtrahendColumn, const QString suffix) :
		CompositeColumn(uiName, Qt::AlignRight),
		minuendColumn(minuendColumn),
		subtrahendColumn(subtrahendColumn),
		suffix(suffix)
{
	assert(minuendColumn->table == subtrahendColumn->table);
	assert(!minuendColumn->isKey() && !subtrahendColumn->isKey());
	assert(minuendColumn->type == subtrahendColumn->type);
	assert(minuendColumn != subtrahendColumn);
	assert(minuendColumn->type == integer || minuendColumn->type == date);
}



QVariant DifferenceCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	QVariant minuendContent = minuendColumn->table->getBufferRow(rowIndex)->at(minuendColumn->getIndex());
	QVariant subtrahendContent = subtrahendColumn->table->getBufferRow(rowIndex)->at(subtrahendColumn->getIndex());
	
	if (!minuendContent.isValid() || !subtrahendContent.isValid()) return QVariant();
	
	switch (minuendColumn->type) {
	case integer: {
		assert(minuendContent.canConvert<int>() && subtrahendContent.canConvert<int>());
		int minuend = minuendContent.toInt();
		int subtrahend = subtrahendContent.toInt();
		
		return minuend - subtrahend;
	}
	case date: {
		assert(minuendContent.canConvert<QDate>() && subtrahendContent.canConvert<QDate>());
		QDate minuend = minuendContent.toDate();
		QDate subtrahend = subtrahendContent.toDate();
		
		int difference = subtrahend.daysTo(minuend);
		return QString::number(difference) + suffix;
	}
	default:
		assert(false);
		return QVariant();
	}
}





DependentEnumCompositeColumn::DependentEnumCompositeColumn(QString uiName, const Column* discerningEnumColumn, const Column* displayedEnumColumn, const QList<QPair<QString, QStringList>>* enumNameLists) :
		CompositeColumn(uiName, Qt::AlignLeft),
		discerningEnumColumn(discerningEnumColumn),
		displayedEnumColumn(displayedEnumColumn),
		enumNameLists(enumNameLists)
{
	assert(discerningEnumColumn->table == displayedEnumColumn->table);
	assert(!discerningEnumColumn->isKey() && !displayedEnumColumn->isKey());
	assert(discerningEnumColumn->type == integer && displayedEnumColumn->type == integer);
	assert(discerningEnumColumn != displayedEnumColumn);
}



QVariant DependentEnumCompositeColumn::data(int rowIndex, int role) const
{
	if (role == Qt::TextAlignmentRole)	return alignment;
	if (role != Qt::DisplayRole)		return QVariant();
	
	QVariant discerningContent = discerningEnumColumn->table->getBufferRow(rowIndex)->at(discerningEnumColumn->getIndex());
	QVariant displayedContent = displayedEnumColumn->table->getBufferRow(rowIndex)->at(displayedEnumColumn->getIndex());
	
	assert(discerningContent.canConvert<int>() && displayedContent.canConvert<int>());
	int discerning = discerningContent.toInt();
	int displayed = displayedContent.toInt();
	
	if (discerning < 1 || displayed < 1) return QVariant();
	
	assert(discerning >= 0 && discerning < enumNameLists->size());
	const QStringList& enumNames = enumNameLists->at(discerning).second;
	assert(displayed >= 0 && displayed < enumNames.size());
	
	return enumNames.at(displayed);
}
