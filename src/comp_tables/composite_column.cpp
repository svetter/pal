#include "composite_column.h"

#include "src/db/normal_table.h"
#include "src/db/table.h"
#include "src/comp_tables/composite_table.h"



CompositeColumn::CompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, DataType contentType, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
		table(table),
		enumNames(enumNames),
		enumNameLists(enumNameLists),
		uiName(uiName),
		alignment(alignment),
		contentType(contentType)
{}

CompositeColumn::~CompositeColumn()
{}



QVariant CompositeColumn::replaceEnumIfApplicable(QVariant content) const
{
	if (!enumNames) return content;
	
	// If content is enumerative, replace index with the corresponding string
	assert(content.canConvert<int>());
	int index = content.toInt();
	assert(index >= 0 && index < enumNames->size());
	return enumNames->at(index);
}



QVariant CompositeColumn::toFormattedTableContent(QVariant rawCellContent) const
{
	if (!rawCellContent.isValid()) return QVariant();
	
	if (contentType == Date) {
		assert(rawCellContent.canConvert<QDate>());
		return rawCellContent.toDate().toString("dd.MM.yyyy");
	}
	
	if (contentType == Time) {
		assert(rawCellContent.canConvert<QTime>());
		return rawCellContent.toTime().toString("HH:mm");
	}
	
	if (contentType == Enum) {
		assert(enumNames);
		assert(rawCellContent.canConvert<int>());
		int enumIndex = rawCellContent.toInt();
		assert(enumIndex >= 0 && enumIndex < enumNames->size());
		return enumNames->at(enumIndex);
	}
	if (contentType == DualEnum) {
		assert(enumNameLists);
		assert(rawCellContent.canConvert<QList<QVariant>>());
		QList<QVariant> intList = rawCellContent.toList();
		assert(intList.size() == 2);
		assert(intList.at(0).canConvert<int>() && intList.at(1).canConvert<int>());
		int discerningEnumIndex	= intList.at(0).toInt();
		int displayedEnumIndex	= intList.at(1).toInt();
		assert(discerningEnumIndex >= 0 && discerningEnumIndex <= enumNameLists->size());
		QList<QString> specifiedEnumNames = enumNameLists->at(discerningEnumIndex).second;
		assert(displayedEnumIndex >= 0 && displayedEnumIndex <= specifiedEnumNames.size());
		return specifiedEnumNames.at(displayedEnumIndex);
	}
	
	return rawCellContent;
}



void CompositeColumn::announceChangedData() const
{
	int thisColumnIndex = table->getIndexOf(this);
	table->announceChangesUnderColumn(thisColumnIndex);
}





DirectCompositeColumn::DirectCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, contentColumn->type, enumNames),
		contentColumn(contentColumn)
{
	assert(contentColumn);
}



QVariant DirectCompositeColumn::getValueAt(int rowIndex) const
{
	QVariant content = contentColumn->getValueAt(rowIndex);
	content = replaceEnumIfApplicable(content);
	
	return content;
}



const QSet<Column* const> DirectCompositeColumn::getAllUnderlyingColumns() const
{
	return { contentColumn };
}





ReferenceCompositeColumn::ReferenceCompositeColumn(CompositeTable* table, QString uiName, Qt::AlignmentFlag alignment, QList<Column*> foreignKeyColumnSequence, Column* contentColumn, const QStringList* enumNames) :
		CompositeColumn(table, uiName, alignment, contentColumn->type, enumNames),
		foreignKeyColumnSequence(foreignKeyColumnSequence),
		contentColumn(contentColumn)
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
		ItemID key = currentColumn->getValueAt(currentRowIndex);
		
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
	QVariant content = contentColumn->getValueAt(currentRowIndex);
	content = replaceEnumIfApplicable(content);
	
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
		CompositeColumn(table, uiName, op == List ? Qt::AlignLeft : Qt::AlignRight, op == List ? String : (op == Count ? Integer : contentColumn->type), enumNames),
		op(op),
		breadcrumbs(breadcrumbs),
		contentColumn(contentColumn)
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
			ItemID key = firstColumn->getValueAt(rowIndex);
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
	
	int aggregate = 0;
	QString listString = "";
	
	for (int rowIndex : currentRowIndexSet) {
		QVariant content = contentColumn->getValueAt(rowIndex);
		
		switch (op) {
		case List:
			assert(content.canConvert<QString>());
			if (!listString.isEmpty()) listString.append(", ");
			content = replaceEnumIfApplicable(content);
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
		CompositeColumn(table, uiName, Qt::AlignRight, Integer),
		minuendColumn(minuendColumn),
		subtrahendColumn(subtrahendColumn),
		suffix(suffix)
{
	assert(minuendColumn && subtrahendColumn);
	assert(minuendColumn->table == subtrahendColumn->table);
	assert(!minuendColumn->isKey() && !subtrahendColumn->isKey());
	assert(minuendColumn->type == subtrahendColumn->type);
	assert(minuendColumn != subtrahendColumn);
	assert(minuendColumn->type == Integer || minuendColumn->type == Date);
}



QVariant DifferenceCompositeColumn::getValueAt(int rowIndex) const
{
	QVariant minuendContent = minuendColumn->getValueAt(rowIndex);
	QVariant subtrahendContent = subtrahendColumn->getValueAt(rowIndex);
	
	if (!minuendContent.isValid() || !subtrahendContent.isValid()) return QVariant();
	
	switch (minuendColumn->type) {
	case Integer: {
		assert(minuendContent.canConvert<int>() && subtrahendContent.canConvert<int>());
		int minuend = minuendContent.toInt();
		int subtrahend = subtrahendContent.toInt();
		
		int difference = minuend - subtrahend;
		return QString::number(difference) + suffix;
	}
	case Date: {
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
		CompositeColumn(table, uiName, Qt::AlignLeft, DualEnum, nullptr, enumNameLists),
		discerningEnumColumn(discerningEnumColumn),
		displayedEnumColumn(displayedEnumColumn)
{
	assert(discerningEnumColumn && displayedEnumColumn);
	assert(discerningEnumColumn->table == displayedEnumColumn->table);
	assert(!discerningEnumColumn->isKey() && !displayedEnumColumn->isKey());
	assert(discerningEnumColumn->type == DualEnum && displayedEnumColumn->type == DualEnum);
	assert(discerningEnumColumn != displayedEnumColumn);
}



QVariant DependentEnumCompositeColumn::getValueAt(int rowIndex) const
{
	QVariant discerningContent = discerningEnumColumn->getValueAt(rowIndex);
	QVariant displayedContent = displayedEnumColumn->getValueAt(rowIndex);
	
	assert(discerningContent.canConvert<int>() && displayedContent.canConvert<int>());
	int discerning = discerningContent.toInt();
	int displayed = displayedContent.toInt();
	
	if (discerning < 1 || displayed < 1) return QVariant();
	
	return QVariant(QList<QVariant>({ discerning, displayed }));
}



const QSet<Column* const> DependentEnumCompositeColumn::getAllUnderlyingColumns() const
{
	return { discerningEnumColumn, displayedEnumColumn };
}
