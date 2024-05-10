/*
 * Copyright 2023-2024 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file composite_column.cpp
 * 
 * This file defines the CompositeColumn class and some of its subclasses.
 */

#include "composite_column.h"

#include "composite_table.h"
#include "fold_composite_column.h"
#include "src/data/enum_names.h"
#include "src/main/item_types_handler.h"



/**
 * Creates a CompositeColumn.
 * 
 * @param table						The CompositeTable that this column belongs to.
 * @param name						The internal name for this column.
 * @param uiName					The name of this column as it should be displayed in the UI.
 * @param contentType				The type of data the column contents.
 * @param cellsAreInterdependent	Whether the contents of the cells in this column depend on each other.
 * @param isStatistical				Whether the column is a statistical column.
 * @param suffix					A suffix to append to the content of each cell.
 * @param enumNames					An optional list of enum names with which to replace the raw cell content.
 * @param enumNameLists				An optional list of enum name lists with which to replace the raw cell content.
 */
CompositeColumn::CompositeColumn(CompColType type, CompositeTable& table, QString name, QString uiName, DataType contentType, bool cellsAreInterdependent, bool isStatistical, QString suffix, const QStringList* enumNames, const QList<QPair<QString, QStringList>>* enumNameLists) :
	type(type),
	table(table),
	name(name),
	uiName(uiName),
	contentType(contentType),
	alignment((contentType == Integer) ? Qt::AlignRight : contentType == Bit ? Qt::AlignCenter : Qt::AlignLeft),
	cellsAreInterdependent(cellsAreInterdependent),
	isStatistical(isStatistical),
	enumNames(enumNames),
	enumNameLists(enumNameLists),
	suffix(suffix)
{
	assert(!cellsAreInterdependent || isStatistical);
}

/**
 * Destroys the CompositeColumn.
 */
CompositeColumn::~CompositeColumn()
{}



/**
 * Formats the raw content of a cell for display in the UI.
 * 
 * Formats integers according to locale, dates and time, replaces enumerative indices with their
 * corresponding strings, and appends the suffix.
 * 
 * @param rawCellContent	The raw cell content.
 * @return					The cell content formatted to string.
 */
QString CompositeColumn::toFormattedTableContent(QVariant rawCellContent) const
{
	if (!rawCellContent.isValid()) return QString();
	
	QString result = rawCellContent.toString();
	
	if (contentType == Integer) {
		assert(rawCellContent.canConvert<int>());
		const int number = rawCellContent.toInt();
		if (abs(number) > 9999) {
			result = QLocale().toString(number);
		}
	}
	
	else if (contentType == Date) {
		assert(rawCellContent.canConvert<QDate>());
		if (!rawCellContent.toDate().isValid()) return QString();
		result = rawCellContent.toDate().toString("dd.MM.yyyy");
	}
	
	else if (contentType == Time) {
		assert(rawCellContent.canConvert<QTime>() && rawCellContent.toTime().isValid());
		result = rawCellContent.toTime().toString("HH:mm");
	}
	
	else if (contentType == Enum) {
		assert(enumNames);
		assert(rawCellContent.canConvert<int>());
		int enumIndex = rawCellContent.toInt();
		if (!(enumIndex >= 0 && enumIndex < enumNames->size()))
		assert(enumIndex >= 0 && enumIndex < enumNames->size());
		result = EnumNames::tr(enumNames->at(enumIndex).toStdString().c_str());
	}
	else if (contentType == DualEnum) {
		assert(enumNameLists);
		assert(rawCellContent.canConvert<QList<QVariant>>());
		const QList<QVariant> intList = rawCellContent.toList();
		assert(intList.size() == 2);
		assert(intList.at(0).canConvert<int>() && intList.at(1).canConvert<int>());
		const int discerningEnumIndex	= intList.at(0).toInt();
		const int displayedEnumIndex	= intList.at(1).toInt();
		assert(discerningEnumIndex >= 0 && discerningEnumIndex <= enumNameLists->size());
		QStringList specifiedEnumNames = EnumNames::translateList(enumNameLists->at(discerningEnumIndex).second);
		specifiedEnumNames.removeAt(1);
		assert(displayedEnumIndex >= 0 && displayedEnumIndex <= specifiedEnumNames.size());
		result = specifiedEnumNames.at(displayedEnumIndex);
	}
	
	return result + suffix;
}



/**
 * Returns the column's index in its table.
 * 
 * @pre This is a normal or filter-only column, not an export-only column.
 * 
 * @return	The column's index.
 */
int CompositeColumn::getIndex() const
{
	return table.getIndexOf(*this);
}

/**
 * Returns the column's index in the export-only column list of its table.
 * 
 * @pre This is an export-only column, not a normal or filter-only column.
 * 
 * @return	The column's index in the export-only column list.
 */
int CompositeColumn::getExportIndex() const
{
	return table.getExportIndexOf(*this);
}

/**
 * Indicates whether this column is only used for exports and not for UI display.
 * 
 * @return	True if this is an export-only column, false otherwise.
 */
bool CompositeColumn::isExportOnlyColumn() const
{
	return table.getExportIndexOf(*this) >= 0;
}

/**
 * Indicates whether this column is only used for filtering and not for UI display.
 * 
 * @return	True if this is a filter-only column, false otherwise.
 */
bool CompositeColumn::isFilterOnlyColumn() const
{
	return getIndex() >= table.getNumberOfNormalColumns();
}



/**
 * Returns a set of valid IDs for all rows in the target (content) table which are associated with
 * the given row of this composite column.
 * 
 * This default implementation assumes that the target table is the same as the start table of this
 * composite column and thus returns the primary key of the base table at the given row.
 * 
 * @param rowIndex	The row index for which to compute the associated target table IDs.
 * @return			A set of target table IDs associated with the given row.
 */
QSet<ValidItemID> CompositeColumn::computeIDsAt(BufferRowIndex rowIndex) const
{
	return { VALID_ITEM_ID(table.baseTable.primaryKeyColumn.getValueAt(rowIndex)) };
}



/**
 * Computes the value of all cells in the column together.
 *
 * This is used for columns with interdependent cells, such as the IndexCompositeColumn.
 *
 * @return	Computed values for all cells in the column.
 */
QList<QVariant> CompositeColumn::computeWholeColumn() const
{
	QList<QVariant> cells = QList<QVariant>();
	for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(table.baseTable.getNumberOfRows()); rowIndex++) {
		cells.append(computeValueAt(rowIndex));
	}
	return cells;
}



/**
 * Returns the raw computed value of the cell at the given row index.
 * 
 * @param rowIndex	The row index.
 * @return			The raw computed value of the cell.
 */
QVariant CompositeColumn::getRawValueAt(BufferRowIndex rowIndex) const
{
	return table.getRawValue(rowIndex, *this);
}

/**
 * Returns the formatted computed value of the cell at the given row index.
 * 
 * @param rowIndex	The row index.
 * @return			The formatted computed value of the cell.
 */
QVariant CompositeColumn::getFormattedValueAt(BufferRowIndex rowIndex) const
{
	return table.getFormattedValue(rowIndex, *this);
}



/**
 * Replaced enumerative indices with their corresponding strings in columns which have enumNames
 * specified.
 * 
 * @param content	The raw cell content.
 * @return			The enum string corresponding to the index in the raw cell content.
 */
QVariant CompositeColumn::replaceEnumIfApplicable(QVariant content) const
{
	if (!enumNames) return content;
	
	// If content is enumerative, replace index with the corresponding string
	assert(content.canConvert<int>());
	int index = content.toInt();
	assert(index >= 0 && index < enumNames->size());
	return EnumNames::tr(enumNames->at(index).toStdString().c_str());
}



/**
 * Compares two cells of this column.
 * 
 * @param value1	The first cell's value.
 * @param value2	The second cell's value.
 * @return			True if the first cell's value is smaller than the second cell's value.
 */
bool CompositeColumn::compare(const QVariant& value1, const QVariant& value2) const
{
	return compareCells(contentType, value1, value2);
}



/**
 * Returns the project settings of the table this column belongs to.
 * 
 * @return	The project settings.
 */
const ProjectSettings& CompositeColumn::getProjectSettings() const
{
	return table.getProjectSettings();
}



QString CompositeColumn::encodeToString(QList<const CompositeColumn*> columns)
{
	QStringList entries = QStringList();
	for (const CompositeColumn* const column : columns) {
		entries.append(column->encodeSingleColumnToString());
	}
	const QString encodedColumns = entries.join(",");
	return encodedColumns;
}

QString CompositeColumn::encodeSingleColumnToString() const
{
	const QString header = CompColTypeNames::getName(type) + "CompositeColumn(";
	
	QStringList parameters = QStringList();
	parameters += encodeString("parentBaseTable_name", table.baseTable.name);
	parameters += encodeString("name", name);
	parameters += encodeString("uiName", uiName);
	parameters += encodeTypeSpecific();
	
	return header + parameters.join(",") + ",)";
}

QList<CompositeColumn*> CompositeColumn::decodeFromString(const QString& encoded, Database& db, const ItemTypesHandler& typesHandler)
{
	QString restOfString = encoded;
	QList<CompositeColumn*> columns = QList<CompositeColumn*>();
	
	while (!restOfString.isEmpty()) {
		CompositeColumn* const parsedColumn = decodeSingleColumnFromString(restOfString, db, typesHandler);
		if (!parsedColumn) {
			qDebug() << "WARNING: Aborted parsing columns from string:" << encoded;
			break;
		}
		
		columns.append(parsedColumn);
		
		if (restOfString.isEmpty()) break;
		if (!restOfString.startsWith(",")) {
			qDebug() << "WARNING: Aborted parsing columns from string:" << encoded;
			break;
		}
		restOfString.remove(0, 1);
	}
	
	return columns;
}

CompositeColumn* CompositeColumn::decodeSingleColumnFromString(QString& restOfEncoding, Database& db, const ItemTypesHandler& typesHandler)
{
	bool ok = false;
	
	const CompColType type = decodeHeader<CompColType>(restOfEncoding, "CompositeColumn", &CompColTypeNames::getType, ok);
	if (!ok) return nullptr;
	
	const NormalTable* const parentBaseTable = decodeTableIdentity(restOfEncoding, "parentBaseTable_name", db, ok);
	if (!ok) return nullptr;
	CompositeTable* parentTable;
	for (const ItemTypeMapper* const mapper : typesHandler.getAllMappers()) {
		if (&mapper->baseTable == parentBaseTable) {
			parentTable = &mapper->compTable;
			break;
		}
	}
	assert(parentTable);
	
	const QString name = decodeString(restOfEncoding, "name", ok);
	if (!ok) return nullptr;
	
	const QString uiName = decodeString(restOfEncoding, "uiName", ok);
	if (!ok) return nullptr;
	
	CompositeColumn* decodedColumn = nullptr;
	
	switch (type) {
	case Direct:			decodedColumn =         DirectCompositeColumn::decodeTypeSpecific(*parentTable, name, uiName, restOfEncoding, db);	break;
	case Reference:			decodedColumn =      ReferenceCompositeColumn::decodeTypeSpecific(*parentTable, name, uiName, restOfEncoding, db);	break;
	case CountFold:			decodedColumn =      CountFoldCompositeColumn::decodeTypeSpecific(*parentTable, name, uiName, restOfEncoding, db);	break;
	case NumericFold:		decodedColumn =    NumericFoldCompositeColumn::decodeTypeSpecific(*parentTable, name, uiName, restOfEncoding, db);	break;
	case ListStringFold:	decodedColumn = ListStringFoldCompositeColumn::decodeTypeSpecific(*parentTable, name, uiName, restOfEncoding, db);	break;
	default: assert(false);
	}
	if (!decodedColumn) return nullptr;
	
	const QString endDelimiter = ")";
	if (!restOfEncoding.startsWith(endDelimiter)) {
		delete decodedColumn;
		return nullptr;
	}
	restOfEncoding.remove(0, endDelimiter.size());
	
	return decodedColumn;
}





/**
 * Creates a DirectCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param contentColumn	The column from which to take the actual cell content.
 */
DirectCompositeColumn::DirectCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const Column& contentColumn) :
	CompositeColumn(Direct, table, name, uiName, contentColumn.type, false, false, suffix, contentColumn.enumNames),
	contentColumn(contentColumn)
{
	assert(contentColumn.type != ID);
	assert(!contentColumn.primaryKey);
	assert(!contentColumn.foreignColumn);
}

/**
 * Creates a DirectCompositeColumn.
 * 
 * @param table			The CompositeTable that this column belongs to.
 * @param suffix		A suffix to append to the content of each cell.
 * @param contentColumn	The column from which to take the actual cell content.
 */
DirectCompositeColumn::DirectCompositeColumn(CompositeTable& table, QString suffix, const Column& contentColumn) :
	DirectCompositeColumn(table, contentColumn.name, contentColumn.uiName, suffix, contentColumn)
{}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DirectCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	return contentColumn.getValueAt(rowIndex);
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> DirectCompositeColumn::getAllUnderlyingColumns() const
{
	return { &contentColumn };
}



QStringList DirectCompositeColumn::encodeTypeSpecific() const
{
	QStringList parameters = QStringList();
	parameters += encodeString("contentColumn_table_name", contentColumn.table.name);
	parameters += encodeString("contentColumn_name", contentColumn.name);
	parameters += encodeString("suffix", suffix);
	return parameters;
}

DirectCompositeColumn* DirectCompositeColumn::decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db)
{
	bool ok = false;
	
	const Column* const contentColumn = decodeColumnIdentity(restOfEncoding, "contentColumn_table_name", "contentColumn_name", db, ok);
	if (!ok) return nullptr;
	
	const QString suffix = decodeString(restOfEncoding, "suffix", ok);
	if (!ok) return nullptr;
	
	return new DirectCompositeColumn(parentTable, name, uiName, suffix, *contentColumn);
}





/**
 * Creates a ReferenceCompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param contentColumn	The column from which to take the actual cell content.
 */
ReferenceCompositeColumn::ReferenceCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const Column& contentColumn) :
	CompositeColumn(Reference, table, name, uiName, contentColumn.type, false, false, suffix, contentColumn.enumNames),
	breadcrumbs((assert(!contentColumn.table.isAssociative), table.crumbsTo((NormalTable&) contentColumn.table))),
	contentColumn(contentColumn)
{
	assert(&table.baseTable == &breadcrumbs.getStartTable());
	assert(&table.baseTable != &contentColumn.table);
	assert(!contentColumn.table.isAssociative);
	assert(&contentColumn.table == &breadcrumbs.getTargetTable());
}



/**
 * Returns a set containing the valid ID for the row in the target (content) table which is
 * associated with the given row of this composite column.
 * 
 * @param rowIndex	The row index for which to compute the associated target table ID.
 * @return			A set containing the target table ID associated with the given row.
 */
QSet<ValidItemID> ReferenceCompositeColumn::computeIDsAt(BufferRowIndex rowIndex) const
{
	BufferRowIndex targetRowIndex = breadcrumbs.evaluateAsForwardChain(rowIndex);
	
	if (Q_UNLIKELY(targetRowIndex.isInvalid())) return {};
	
	return { VALID_ITEM_ID(contentColumn.getValueAt(targetRowIndex)) };
}

/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant ReferenceCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	BufferRowIndex targetRowIndex = breadcrumbs.evaluateAsForwardChain(rowIndex);
	
	if (Q_UNLIKELY(targetRowIndex.isInvalid())) return QVariant();
	
	if (contentColumn.primaryKey) {
		return ((const NormalTable&) contentColumn.table).getIdentityRepresentationAt(targetRowIndex);
	}
	
	return contentColumn.getValueAt(targetRowIndex);
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> ReferenceCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<const Column*> result = { &contentColumn };
	result.unite(breadcrumbs.getColumnSet());
	return result;
}



QStringList ReferenceCompositeColumn::encodeTypeSpecific() const
{
	QStringList parameters = QStringList();
	parameters += encodeString("contentColumn_table_name", contentColumn.table.name);
	parameters += encodeString("contentColumn_name", contentColumn.name);
	parameters += encodeString("suffix", suffix);
	return parameters;
}

ReferenceCompositeColumn* ReferenceCompositeColumn::decodeTypeSpecific(CompositeTable& parentTable, const QString& name, const QString& uiName, QString& restOfEncoding, Database& db)
{
	bool ok = false;
	
	const Column* const contentColumn = decodeColumnIdentity(restOfEncoding, "contentColumn_table_name", "contentColumn_name", db, ok);
	if (!ok) return nullptr;
	
	const QString suffix = decodeString(restOfEncoding, "suffix", ok);
	if (!ok) return nullptr;
	
	return new ReferenceCompositeColumn(parentTable, name, uiName, suffix, *contentColumn);
}





/**
 * Creates a DifferenceCompositeColumn.
 *
 * @param table				The CompositeTable that this column belongs to.
 * @param name				The internal name for this column.
 * @param uiName			The name of this column as it should be displayed in the UI.
 * @param suffix			A suffix to append to the content of each cell.
 * @param minuendColumn		The column from which to take the minuends.
 * @param subtrahendColumn	The column from which to take the subtrahends.
 */
DifferenceCompositeColumn::DifferenceCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, ValueColumn& minuendColumn, ValueColumn& subtrahendColumn) :
	CompositeColumn(Difference, table, name, uiName, Integer, false, true, suffix),
	minuendColumn(minuendColumn),
	subtrahendColumn(subtrahendColumn)
{
	assert(&table.baseTable == &minuendColumn.table);
	assert(&minuendColumn.table == &subtrahendColumn.table);
	assert(!minuendColumn.isKey() && !subtrahendColumn.isKey());
	assert(minuendColumn.type == subtrahendColumn.type);
	assert(&minuendColumn != &subtrahendColumn);
	assert(minuendColumn.type == Integer || minuendColumn.type == Date);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DifferenceCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QVariant minuendContent = minuendColumn.getValueAt(rowIndex);
	QVariant subtrahendContent = subtrahendColumn.getValueAt(rowIndex);
	
	if (Q_UNLIKELY(!minuendContent.isValid() || !subtrahendContent.isValid())) return QVariant();
	
	switch (minuendColumn.type) {
	case Integer: {
		assert(minuendContent.canConvert<int>() && subtrahendContent.canConvert<int>());
		const int minuend = minuendContent.toInt();
		const int subtrahend = subtrahendContent.toInt();
		
		return minuend - subtrahend;
	}
	case Date: {
		assert(minuendContent.canConvert<QDate>() && subtrahendContent.canConvert<QDate>());
		const QDate minuend = minuendContent.toDate();
		const QDate subtrahend = subtrahendContent.toDate();
		if (!minuend.isValid() || !subtrahend.isValid())
			return QVariant();
		
		return subtrahend.daysTo(minuend) + 1;
	}
	default:
		assert(false);
		return QVariant();
	}
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> DifferenceCompositeColumn::getAllUnderlyingColumns() const
{
	return { &minuendColumn, &subtrahendColumn };
}



QStringList DifferenceCompositeColumn::encodeTypeSpecific() const
{
	// Not supported
	assert(false);
	return {};
}





/**
 * Creates a DependentEnumCompositeColumn.
 *
 * @param table					The CompositeTable that this column belongs to.
 * @param name					The internal name for this column.
 * @param uiName				The name of this column as it should be displayed in the UI.
 * @param discerningEnumColumn	The column from which to take the discerning enum.
 * @param displayedEnumColumn	The column from which to take the displayed enum.
 */
DependentEnumCompositeColumn::DependentEnumCompositeColumn(CompositeTable& table, QString name, QString uiName, ValueColumn& discerningEnumColumn, ValueColumn& displayedEnumColumn) :
	CompositeColumn(DependentEnum, table, name, uiName, DualEnum, false, false, QString(), nullptr, discerningEnumColumn.enumNameLists),
	discerningEnumColumn(discerningEnumColumn),
	displayedEnumColumn(displayedEnumColumn)
{
	assert(&table.baseTable == &discerningEnumColumn.table);
	assert(&discerningEnumColumn.table == &displayedEnumColumn.table);
	assert(!discerningEnumColumn.isKey() && !displayedEnumColumn.isKey());
	assert(discerningEnumColumn.type == DualEnum && displayedEnumColumn.type == DualEnum);
	assert(discerningEnumColumn.enumNameLists == displayedEnumColumn.enumNameLists);
	assert(&discerningEnumColumn != &displayedEnumColumn);
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant DependentEnumCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	QVariant discerningContent = discerningEnumColumn.getValueAt(rowIndex);
	QVariant displayedContent = displayedEnumColumn.getValueAt(rowIndex);
	
	assert(discerningContent.canConvert<int>() && displayedContent.canConvert<int>());
	int discerning = discerningContent.toInt();
	int displayed = displayedContent.toInt();
	
	if (Q_UNLIKELY(discerning < 1 || displayed < 1)) return QVariant();
	
	return QVariant(QList<QVariant>({ discerning, displayed }));
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> DependentEnumCompositeColumn::getAllUnderlyingColumns() const
{
	return { &discerningEnumColumn, &displayedEnumColumn };
}



QStringList DependentEnumCompositeColumn::encodeTypeSpecific() const
{
	// Not supported
	assert(false);
	return {};
}





/**
 * Creates a CompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param sortingPasses	The list of sorting passes in order of priority, each containing column and order.
 */
IndexCompositeColumn::IndexCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const QList<BaseSortingPass> sortingPasses, bool isOrdinal) :
	CompositeColumn(isOrdinal ? Ordinal : Index, table, name, uiName, Integer, true, true, suffix),
	sortingPasses(sortingPasses)
{
	assert(!sortingPasses.isEmpty());
	for (const auto& [column, order] : sortingPasses) {
		assert(&column.table == &table.baseTable);
		assert(order == Qt::AscendingOrder || order == Qt::DescendingOrder);
	}
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant IndexCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	qDebug() << "CAUTION: Using extremely inefficient IndexCompositeColumn::computeValueAt(BufferRowIndex)";
	QList<BufferRowIndex> order = getRowIndexOrderList();
	return order.indexOf(rowIndex) + 1;
}

/**
 * Computes the value of all cells in the column together.
 *
 * This is used for columns with interdependent cells, such as the IndexCompositeColumn.
 *
 * @return	Computed values for all cells in the column.
 */
QList<QVariant> IndexCompositeColumn::computeWholeColumn() const
{
	QList<BufferRowIndex> order = getRowIndexOrderList();
	
	QList<QVariant> cells = QList<QVariant>();
	for (BufferRowIndex rowIndex = BufferRowIndex(0); rowIndex.isValid(order.size()); rowIndex++) {
		cells.append(order.indexOf(rowIndex) + 1);
	}
	
	return cells;
}

QList<BufferRowIndex> IndexCompositeColumn::getRowIndexOrderList() const
{
	int numberOfRows = sortingPasses.at(0).column.table.getNumberOfRows();
	// Local order buffer which represents the ordered list of row indices
	QList<BufferRowIndex> order = QList<BufferRowIndex>();
	for (BufferRowIndex index = BufferRowIndex(0); index.isValid(numberOfRows); index++) {
		order.append(index);
	}
	
	for (int i = sortingPasses.size() - 1; i >= 0; i--) {
		const BaseSortingPass sorting = sortingPasses.at(i);
		
		auto comparator = [&sorting](BufferRowIndex i1, BufferRowIndex i2) {
			QVariant value1 = sorting.column.getValueAt(i1);
			QVariant value2 = sorting.column.getValueAt(i2);
			
			if (sorting.order == Qt::AscendingOrder) {
				return compareCells(sorting.column.type, value1, value2);
			} else {
				return compareCells(sorting.column.type, value2, value1);
			}
		};
		
		std::stable_sort(order.begin(), order.end(), comparator);
	}
	
	return order;
}



/**
 * Returns a set of all columns in the base tables which are used to compute the content of
 * this column.
 *
 * @return	A set of all base table columns which are used to compute contents of this column.
 */
const QSet<const Column*> IndexCompositeColumn::getAllUnderlyingColumns() const
{
	QSet<const Column*> columns = QSet<const Column*>();
	for (const auto& [column, order] : sortingPasses) {
		columns += &column;
	}
	return columns;
}



QStringList IndexCompositeColumn::encodeTypeSpecific() const
{
	// Not supported
	assert(false);
	return {};
}





/**
 * Creates a CompositeColumn.
 *
 * @param table			The CompositeTable that this column belongs to.
 * @param name			The internal name for this column.
 * @param uiName		The name of this column as it should be displayed in the UI.
 * @param suffix		A suffix to append to the content of each cell.
 * @param sortingPasses	The list of columns to sort by and their sort order, in order of priority. The first column automatically doubles as the separating (grouping) column.
 */
OrdinalCompositeColumn::OrdinalCompositeColumn(CompositeTable& table, QString name, QString uiName, QString suffix, const QList<BaseSortingPass> sortingPasses) :
	IndexCompositeColumn(table, name, uiName, suffix, sortingPasses, true),
	separatingColumn(sortingPasses.first().column)
{
	assert(sortingPasses.first().column.isForeignKey());
}



/**
 * Computes the value of the cell at the given row index.
 *
 * @param rowIndex	The row index.
 * @return			The computed value of the cell.
 */
QVariant OrdinalCompositeColumn::computeValueAt(BufferRowIndex rowIndex) const
{
	qDebug() << "CAUTION: Using extremely inefficient OrdinalCompositeColumn::computeValueAt(BufferRowIndex)";
	return computeWholeColumn().at(rowIndex.get());
}

/**
 * Computes the value of all cells in the column together.
 *
 * This is used for columns with interdependent cells, such as the IndexCompositeColumn.
 *
 * @return	Computed values for all cells in the column.
 */
QList<QVariant> OrdinalCompositeColumn::computeWholeColumn() const
{
	QList<BufferRowIndex> order = getRowIndexOrderList();
	
	QList<QVariant> ordinals = QList<QVariant>(order.size());
	
	ItemID lastKey = ItemID();
	int ordinal = 1;
	for (const BufferRowIndex& rowIndex : order) {
		ItemID currentKey = separatingColumn.getValueAt(rowIndex);
		if (Q_UNLIKELY(!currentKey.isValid())) {
			// No key, reset ordinal and append empty
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex.get(), QVariant());
		} else if (Q_LIKELY(currentKey != lastKey)) {
			// Next key, reset ordinal
			ordinal = 1;
			lastKey = currentKey;
			ordinals.replace(rowIndex.get(), ordinal);
		} else {
			// Same key, increase ordinal
			ordinal++;
			ordinals.replace(rowIndex.get(), ordinal);
		}
	}
	
	return ordinals;
}



QStringList OrdinalCompositeColumn::encodeTypeSpecific() const
{
	// Not supported
	assert(false);
	return {};
}
