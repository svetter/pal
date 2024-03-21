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



Filter::Filter(DataType type, const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name) :
	type(type),
	tableToFilter(tableToFilter),
	columnToFilterBy(columnToFilterBy),
	name(name),
	enabled(true),
	inverted(false)
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



QString Filter::encodeToString() const
{
	const QString header = DataTypeNames::getName(type) + "Filter(";
	
	QStringList parameters = QStringList();
	parameters += encodeString	("tableToFilter_name",			tableToFilter.name);
	parameters += encodeString	("columnToFilterBy_table_name",	columnToFilterBy.table.name);
	parameters += encodeString	("columnToFilterBy_name",		columnToFilterBy.name);
	parameters += encodeString	("name",						name);
	parameters += encodeBool	("enabled",						enabled);
	parameters += encodeBool	("inverted",					inverted);
	parameters += encodeTypeSpecific();
	
	return header + parameters.join(",");
}

unique_ptr<Filter> Filter::decodeFromString(const QString& encoded, Database& db)
{
	QString restOfString = encoded;
	bool ok = false;
	
	const DataType type = decodeHeader(restOfString, ok);
	if (!ok) return nullptr;
	
	const NormalTable* tableToFilter = decodeTableIdentity(restOfString, "tableToFilter_name", ok, db);
	if (!ok) return nullptr;
	
	const Column* columnToFilterBy = decodeColumnIdentity(restOfString, "columnToFilterBy_table_name", "columnToFilterBy_name", ok, db);
	if (!ok) return nullptr;
	
	const QString name = decodeString(restOfString, "name", ok);
	if (!ok) return nullptr;
	
	const bool enabled = decodeBool(restOfString, "enabled", ok);
	if (!ok) return nullptr;
	
	const bool inverted = decodeBool(restOfString, "inverted", ok);
	if (!ok) return nullptr;
	
	unique_ptr<Filter> filter = nullptr;
	
	switch (type) {
	case Integer:	filter =      IntFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	case ID:		filter =       IDFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	case Enum:		filter =     EnumFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	case DualEnum:	filter = DualEnumFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	case Bit:		filter =     BoolFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	case String:	filter =   StringFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	case Date:		filter =     DateFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	case Time:		filter =     TimeFilter::decodeTypeSpecific(*tableToFilter, *columnToFilterBy, name, restOfString);	break;
	default: assert(false);
	}
	
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
	return paramName + "=" + value.toHtmlEscaped();
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
	restOfString.remove(0, valueLength);
	
	ok = true;
	return result;
}

ItemID Filter::decodeID(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	const int parsedInt  = decodeInt(restOfString, paramName, lastParam, ok);
	
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
	restOfString.remove(0, valueLength);
	
	ok = true;
	return result;
}

QString Filter::decodeString(QString& restOfString, const QString& paramName, bool& ok, bool lastParam)
{
	auto fail = [&]() { ok = false; return QString(); };
	
	const QString expectedStart = paramName + "=";
	if (!restOfString.startsWith(expectedStart)) return fail();
	restOfString.remove(0, expectedStart.size());
	
	const QString endDelimiter = QString("\"") + (lastParam ? ")" : ",");
	const int valueLength = restOfString.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfString.first(valueLength);
	const QString name = fromHtmlEscaped(valueString);
	restOfString.remove(0, valueString.size());
	
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
	restOfString.remove(0, valueLength);
	
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
	restOfString.remove(0, valueLength);
	
	ok = true;
	return result;
}
