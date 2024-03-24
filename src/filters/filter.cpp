#include "filter.h"

#include "src/db/database.h"
#include "src/filters/int_filter.h"
#include "src/filters/id_filter.h"
#include "src/filters/enum_filter.h"
#include "src/filters/dual_enum_filter.h"
#include "src/filters/bool_filter.h"
#include "src/filters/string_filter.h"
#include "src/filters/date_filter.h"
#include "src/filters/time_filter.h"
#include "src/main/helpers.h"

#include <QDateTime>



Filter::Filter(DataType type, const NormalTable& tableToFilter, const Column& columnToFilterBy, FilterFoldOp foldOp, const QString& name) :
	type(type),
	sourceType(columnToFilterBy.type),
	tableToFilter(tableToFilter),
	columnToFilterBy(columnToFilterBy),
	foldOp(foldOp),
	crumbs(tableToFilter.db.getBreadcrumbsFor(tableToFilter, (NormalTable&) columnToFilterBy.table)),
	isLocalFilter(&columnToFilterBy.table == &tableToFilter),
	singleValuePerRow(crumbs.isEmpty() || crumbs.isForwardOnly()),
	name(name),
	enabled(true),
	inverted(false)
{
	assert(!columnToFilterBy.table.isAssociative);
	assert(isLocalFilter == crumbs.isEmpty());
	if (singleValuePerRow) {
		assert(foldOp == FilterFoldOp(-1));
		assert(type == sourceType);
	} else {
		assert(foldOp != FilterFoldOp(-1));
		if (foldOp == FilterFoldOp_StringList) {
			assert(type == String);
			assert(sourceType == String);
		} else if (foldOp == FilterFoldOp_Count) {
			assert(type == Integer);
		} else {
			assert(type == Integer);
			assert(sourceType == Integer);
		}
	}
}

Filter::~Filter()
{}



bool Filter::isEnabled() const
{
	return enabled;
}

bool Filter::isInverted() const
{
	return inverted;
}

void Filter::setEnabled(bool enabled)
{
	this->enabled = enabled;
}

void Filter::setInverted(bool inverted)
{
	this->inverted = inverted;
}



void Filter::applyToOrderBuffer(ViewOrderBuffer& viewOrderBuffer) const
{
	for (ViewRowIndex viewRow = ViewRowIndex(viewOrderBuffer.numRows() - 1); viewRow.isValid(); viewRow--) {
		const BufferRowIndex filteredTableBufferRow = viewOrderBuffer.getBufferRowIndexForViewRow(viewRow);
		
		// Get raw value
		QVariant rawRowValue = getRawRowValue(filteredTableBufferRow);
		
		// Evaluate filter
		const bool pass = evaluate(rawRowValue);
		
		// Remove row if no match
		if (!pass) {
			viewOrderBuffer.removeViewRow(viewRow);
		}
	}
}

QVariant Filter::getRawRowValue(const BufferRowIndex filteredTableBufferRow) const
{
	if (isLocalFilter) {
		// Column to filter by is in the same table which is being filtered
		const QVariant& firstValue = columnToFilterBy.getValueAt(filteredTableBufferRow);
		
		if (type == DualEnum) {
			const Column& secondColumn = ((DualEnumFilter*) this)->dependentColumnToFilterBy;
			const QVariant& secondValue	= secondColumn.getValueAt(filteredTableBufferRow);
			const QVariant pair = QVariantList({firstValue, secondValue});
			
			return pair;
		}
		
		return firstValue;
	}
	
	// Column to filter by is in a different table than the one being filtered
	// Evaluate breadcrumbs
	QSet<BufferRowIndex> associatedRows = crumbs.evaluate(filteredTableBufferRow);
	
	if (associatedRows.isEmpty()) {
		assert(columnToFilterBy.nullable || !singleValuePerRow);
		
		return QVariant();
	}
	
	if (singleValuePerRow) {
		// There cannot be more than one entry
		assert(associatedRows.size() == 1);
		const BufferRowIndex& foreignBufferRow = *associatedRows.begin();
		const QVariant& firstValue = columnToFilterBy.getValueAt(foreignBufferRow);
		
		if (type == DualEnum) {
			const Column& secondColumn = ((DualEnumFilter*) this)->dependentColumnToFilterBy;
			const QVariant& secondValue	= secondColumn.getValueAt(foreignBufferRow);
			const QVariant pair = QVariantList({firstValue, secondValue});
			
			return pair;
		}
		
		return firstValue;
	}
	// Fold operation needs to be performed
	assert(sourceType != DualEnum);
	
	if (foldOp == FilterFoldOp_Count) {
		return associatedRows.count();
	}
	
	if (foldOp == FilterFoldOp_StringList) {
		assert(sourceType == String);
		QStringList entries = QStringList();
		for (const BufferRowIndex& associatedRow : associatedRows) {
			const QVariant& associatedRowValue = columnToFilterBy.getValueAt(associatedRow);
			assert(associatedRowValue.canConvert<QString>());
			entries.append(associatedRowValue.toString());
		}
		return entries.join(", ");
	}
	
	// Aggregators for different fold operations
	int sum = 0;
	int min = INT_MAX;
	int max = INT_MIN;
	
	// Fold operation loop
	for (const BufferRowIndex& associatedRow : associatedRows) {
		const QVariant& associatedRowValue = columnToFilterBy.getValueAt(associatedRow);
		assert(associatedRowValue.canConvert<int>());
		const int convertedValue = associatedRowValue.toInt();
		
		switch (foldOp) {
		case FilterFoldOp_Sum:
		case FilterFoldOp_Average:
			sum += convertedValue;
			break;
		case FilterFoldOp_Min:
			min = std::min(min, convertedValue);
			break;
		case FilterFoldOp_Max:
			max = std::max(max, convertedValue);
			break;
		default:
			assert(false);
		}
	}
	
	if (foldOp == FilterFoldOp_Sum) {
		return sum;
	}
	
	if (foldOp == FilterFoldOp_Average) {
		return (double) sum / associatedRows.count();
	}
	
	assert(false);
	return QVariant();
}



QString Filter::encodeToString(QList<const Filter*> filters)
{
	QStringList entries = QStringList();
	for (const Filter* const filter : filters) {
		entries.append(filter->encodeSingleFilterToString());
	}
	const QString encodedFilters = entries.join(",");
	return encodedFilters;
}

QString Filter::encodeSingleFilterToString() const
{
	const QString header = DataTypeNames::getName(type) + "Filter(";
	
	QStringList parameters = QStringList();
	parameters += encodeString	("tableToFilter_name",			tableToFilter.name);
	parameters += encodeString	("columnToFilterBy_table_name",	columnToFilterBy.table.name);
	parameters += encodeString	("columnToFilterBy_name",		columnToFilterBy.name);
	if (type == Integer || type == String) {
		parameters += encodeString	("foldOp",					FilterFoldOpNames::getName(foldOp));
	}
	parameters += encodeString	("name",						name);
	parameters += encodeBool	("enabled",						enabled);
	parameters += encodeBool	("inverted",					inverted);
	parameters += encodeTypeSpecific();
	
	return header + parameters.join(",") + ")";
}

QList<Filter*> Filter::decodeFromString(const QString& encoded, Database& db)
{
	QString restOfString = encoded;
	QList<Filter*> filters = QList<Filter*>();
	
	while (!restOfString.isEmpty()) {
		Filter* const parsedFilter = decodeSingleFilterFromString(restOfString, db);
		if (!parsedFilter) {
			qDebug() << "WARNING: Aborted parsing filters from string:" << encoded;
			break;
		}
		
		filters.append(parsedFilter);
		
		if (restOfString.isEmpty()) break;
		if (!restOfString.startsWith(",")) {
			qDebug() << "WARNING: Aborted parsing filters from string:" << encoded;
			break;
		}
		restOfString.remove(0, 1);
	}
	
	return filters;
}

Filter* Filter::decodeSingleFilterFromString(QString& restOfString, Database& db)
{
	bool ok = false;
	
	const DataType type = decodeHeader(restOfString, ok);
	if (!ok) return nullptr;
	
	const NormalTable* tableToFilter = decodeTableIdentity(restOfString, "tableToFilter_name", ok, db);
	if (!ok) return nullptr;
	
	const Column* columnToFilterBy = decodeColumnIdentity(restOfString, "columnToFilterBy_table_name", "columnToFilterBy_name", ok, db);
	if (!ok) return nullptr;
	
	FilterFoldOp foldOp = FilterFoldOp(-1);
	if (type == Integer || type == String) {
		foldOp = FilterFoldOpNames::getFoldOp(decodeString(restOfString, "foldOp", ok));
		if (!ok) return nullptr;
	}
	
	const QString name = decodeString(restOfString, "name", ok);
	if (!ok) return nullptr;
	
	const bool enabled = decodeBool(restOfString, "enabled", ok);
	if (!ok) return nullptr;
	
	const bool inverted = decodeBool(restOfString, "inverted", ok);
	if (!ok) return nullptr;
	
	Filter* filter = nullptr;
	
	switch (type) {
	case Integer:	filter =      IntFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, foldOp,	name, restOfString);	break;
	case ID:		filter =       IDFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy,			name, restOfString);	break;
	case Enum:		filter =     EnumFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy,			name, restOfString);	break;
	case DualEnum:	filter = DualEnumFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy,			name, restOfString);	break;
	case Bit:		filter =     BoolFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy,			name, restOfString);	break;
	case String:	filter =   StringFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, foldOp,	name, restOfString);	break;
	case Date:		filter =     DateFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy,			name, restOfString);	break;
	case Time:		filter =     TimeFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy,			name, restOfString);	break;
	default: assert(false);
	}
	
	if (!filter) return nullptr;
	
	filter->enabled = enabled;
	filter->inverted = inverted;
	
	return filter;
}



QString Filter::encodeInt(const QString& paramName, int value)
{
	return paramName + "=" + QString::number(value);
}

QString Filter::encodeID(const QString& paramName, const ItemID& value)
{
	return paramName + "=" + value.asQVariant().toString();
}

QString Filter::encodeBool(const QString& paramName, bool value)
{
	return paramName + "=" + (value ? "true" : "false");
}

QString Filter::encodeString(const QString& paramName, const QString& value)
{
	return paramName + "=\"" + value.toHtmlEscaped() + "\"";
}

QString Filter::encodeDate(const QString& paramName, const QDate& value)
{
	return paramName + "=" + value.toString(Qt::ISODate);
}

QString Filter::encodeTime(const QString& paramName, const QTime& value)
{
	return paramName + "=" + value.toString(Qt::ISODate);
}



DataType Filter::decodeHeader(QString& restOfString, bool& ok)
{
	auto fail = [&]() { ok = false; return DataType(-1); };
	
	const QString delimiter = "Filter(";
	const int typeNameLength = restOfString.indexOf(delimiter);
	if (typeNameLength < 0) return fail();
	const QString filterTypeString = restOfString.first(typeNameLength);
	const DataType type = DataTypeNames::getType(filterTypeString);
	restOfString.remove(0, typeNameLength + delimiter.size());
	
	ok = true;
	return type;
}

const NormalTable* Filter::decodeTableIdentity(QString& restOfString, const QString& tableNameParamName, bool& ok, Database& db)
{
	const QString tableName = decodeString(restOfString, tableNameParamName, ok);
	if (!ok) return nullptr;
	
	for (const NormalTable* const table : db.getNormalItemTableList()) {
		if (table->name == tableName) {
			return table;
		}
	}
	return nullptr;
}

const Column* Filter::decodeColumnIdentity(QString& restOfString, const QString& tableNameParamName, const QString& columnNameParamName, bool& ok, Database& db)
{
	const NormalTable* table = decodeTableIdentity(restOfString, tableNameParamName, ok, db);
	if (!ok) return nullptr;
	
	const QString columnName = decodeString(restOfString, columnNameParamName, ok);
	if (!ok) return nullptr;
	
	for (const Column* const column : table->getColumnList()) {
		if (column->name == columnName) {
			return column;
		}
	}
	return nullptr;
}

int Filter::decodeInt(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	auto fail = [&]() { ok = false; return -1; };
	
	const QString expectedStart = paramName + "=";
	if (!restOfString.startsWith(expectedStart)) return fail();
	restOfString.remove(0, expectedStart.size());
	
	const QString endDelimiter = lastParam ? ")" : ",";
	const int valueLength = restOfString.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfString.first(valueLength);
	const int result = valueString.toInt(&ok);
	if (!ok) return fail();
	restOfString.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}

ItemID Filter::decodeID(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	const int parsedInt = decodeInt(restOfString, paramName, ok, lastParam);
	
	if (!ok) return ItemID(-1);
	return ItemID(parsedInt);
}

bool Filter::decodeBool(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	auto fail = [&]() { ok = false; return false; };
	
	const QString expectedStart = paramName + "=";
	if (!restOfString.startsWith(expectedStart)) return fail();
	restOfString.remove(0, expectedStart.size());
	
	const QString endDelimiter = lastParam ? ")" : ",";
	const int valueLength = restOfString.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfString.first(valueLength);
	if (valueString != "true" && valueString != "false") return fail();
	const bool result = valueString == "true";
	restOfString.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}

QString Filter::decodeString(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	auto fail = [&]() { ok = false; return QString(); };
	
	const QString expectedStart = paramName + "=\"";
	if (!restOfString.startsWith(expectedStart)) return fail();
	restOfString.remove(0, expectedStart.size());
	
	const QString endDelimiter = QString("\"") + (lastParam ? ")" : ",");
	const int valueLength = restOfString.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfString.first(valueLength);
	const QString name = fromHtmlEscaped(valueString);
	restOfString.remove(0, valueString.size() + endDelimiter.length());
	
	ok = true;
	return name;
}

QDate Filter::decodeDate(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	auto fail = [&]() { ok = false; return QDate(); };
	
	const QString expectedStart = paramName + "=";
	if (!restOfString.startsWith(expectedStart)) return fail();
	restOfString.remove(0, expectedStart.size());
	
	const QString endDelimiter = lastParam ? ")" : ",";
	const int valueLength = restOfString.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfString.first(valueLength);
	const QDate result = QDate::fromString(valueString, Qt::ISODate);
	if (!result.isValid()) return fail();
	restOfString.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}

QTime Filter::decodeTime(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	auto fail = [&]() { ok = false; return QTime(); };
	
	const QString expectedStart = paramName + "=";
	if (!restOfString.startsWith(expectedStart)) return fail();
	restOfString.remove(0, expectedStart.size());
	
	const QString endDelimiter = lastParam ? ")" : ",";
	const int valueLength = restOfString.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfString.first(valueLength);
	const QTime result = QTime::fromString(valueString, Qt::ISODate);
	if (!result.isValid()) return fail();
	restOfString.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}
