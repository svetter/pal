#include "composite_column.h"

#include "src/db/normal_table.h"
#include "src/db/table.h"
#include "src/comp_tables/composite_table.h"



CompositeColumn::CompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType) :
		table(table),
		uiName(uiName),
		alignment(alignment),
		contentType(contentType)
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



void CompositeColumn::announceChangedData() const
{
	int thisColumnIndex = table->getIndexOf(this);
	table->announceChangesUnderColumn(thisColumnIndex);
}





DirectCompositeColumn::DirectCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, enumNames ? varchar : contentColumn->type),
		contentColumn(contentColumn),
		enumNames(enumNames)
{
	assert(contentColumn);
}



QVariant DirectCompositeColumn::getValueAt(int rowIndex) const
{
	QVariant content = contentColumn->table->getBufferRow(rowIndex)->at(contentColumn->getIndex());
	content = replaceEnumIfApplicable(content, enumNames);
	
	return content;
}



const QSet<Column* const> DirectCompositeColumn::getAllUnderlyingColumns() const
{
	return { contentColumn };
}





ReferenceCompositeColumn::ReferenceCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, QList<Column*> foreignKeyColumnSequence, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, enumNames ? varchar : contentColumn->type),
		foreignKeyColumnSequence(foreignKeyColumnSequence),
		contentColumn(contentColumn),
		enumNames(enumNames)
{
	assert(contentColumn);
}



QVariant ReferenceCompositeColumn::getValueAt(int rowIndex) const
{
	assert(foreignKeyColumnSequence.first()->isForeignKey());
	
	int currentRowIndex = rowIndex;
	assert(!foreignKeyColumnSequence.first()->table->isAssociative);
	NormalTable* currentTable = (NormalTable*) foreignKeyColumnSequence.first()->table;
	
	for (const Column* currentColumn : foreignKeyColumnSequence) {
		assert(currentColumn->table == currentTable);
		assert(currentColumn->isForeignKey());
		
		// Look up key stored in current column at current row index
		ItemID key = currentTable->getBufferRow(currentRowIndex)->at(currentColumn->getIndex());
		
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



const QSet<Column* const> ReferenceCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> result = { contentColumn };
	for (Column* column : foreignKeyColumnSequence) {
		result.insert(column);
		result.insert(column->getReferencedForeignColumn());
	}
	return result;
}





FoldCompositeColumn::FoldCompositeColumn(CompositeTable* table, QString uiName, FoldOp op, const QList<QPair<Column*, Column*>> breadcrumbs, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, op == List ? Qt::AlignLeft : Qt::AlignRight, (op == List || enumNames) ? varchar : (op == Count ? integer : contentColumn->type)),
		op(op),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn),
		enumNames(enumNames)
{
	assert((op == Count) == (contentColumn == nullptr));
}



QVariant FoldCompositeColumn::getValueAt(int rowIndex) const
{
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
			ItemID key = currentTable->getBufferRow(rowIndex)->at(firstColumn->getIndex());
			if (key.isValid()) currentKeySet.insert(key.forceValid());
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



const QSet<Column* const> FoldCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<Column* const> result = QSet<Column* const>();
	if (contentColumn) result.insert(contentColumn);
	for (QPair<Column*, Column*> pair : breadcrumbs) {
		result.insert(pair.first);
		result.insert(pair.second);
	}
	return result;
}





DifferenceCompositeColumn::DifferenceCompositeColumn(CompositeTable* table, QString uiName, Column* minuendColumn, Column* subtrahendColumn, const QString suffix) :
		CompositeColumn(table, uiName, Qt::AlignRight, integer),
		minuendColumn(minuendColumn),
		subtrahendColumn(subtrahendColumn),
		suffix(suffix)
{
	assert(minuendColumn && subtrahendColumn);
	assert(minuendColumn->table == subtrahendColumn->table);
	assert(!minuendColumn->isKey() && !subtrahendColumn->isKey());
	assert(minuendColumn->type == subtrahendColumn->type);
	assert(minuendColumn != subtrahendColumn);
	assert(minuendColumn->type == integer || minuendColumn->type == date);
}



QVariant DifferenceCompositeColumn::getValueAt(int rowIndex) const
{
	QVariant minuendContent = minuendColumn->table->getBufferRow(rowIndex)->at(minuendColumn->getIndex());
	QVariant subtrahendContent = subtrahendColumn->table->getBufferRow(rowIndex)->at(subtrahendColumn->getIndex());
	
	if (!minuendContent.isValid() || !subtrahendContent.isValid()) return QVariant();
	
	switch (minuendColumn->type) {
	case integer: {
		assert(minuendContent.canConvert<int>() && subtrahendContent.canConvert<int>());
		int minuend = minuendContent.toInt();
		int subtrahend = subtrahendContent.toInt();
		
		int difference = minuend - subtrahend;
		return QString::number(difference) + suffix;
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



const QSet<Column* const> DifferenceCompositeColumn::getAllUnderlyingColumns() const
{
	return { minuendColumn, subtrahendColumn };
}





DependentEnumCompositeColumn::DependentEnumCompositeColumn(CompositeTable* table, QString uiName, Column* discerningEnumColumn, Column* displayedEnumColumn, const QList<QPair<QString, QStringList>>* enumNameLists) :
		CompositeColumn(table, uiName, Qt::AlignLeft, varchar),
		discerningEnumColumn(discerningEnumColumn),
		displayedEnumColumn(displayedEnumColumn),
		enumNameLists(enumNameLists)
{
	assert(discerningEnumColumn && displayedEnumColumn);
	assert(discerningEnumColumn->table == displayedEnumColumn->table);
	assert(!discerningEnumColumn->isKey() && !displayedEnumColumn->isKey());
	assert(discerningEnumColumn->type == integer && displayedEnumColumn->type == integer);
	assert(discerningEnumColumn != displayedEnumColumn);
}



QVariant DependentEnumCompositeColumn::getValueAt(int rowIndex) const
{
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



const QSet<Column* const> DependentEnumCompositeColumn::getAllUnderlyingColumns() const
{
	return { discerningEnumColumn, displayedEnumColumn };
}
