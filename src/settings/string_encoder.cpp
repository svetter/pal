/*
 * Copyright 2023-2025 Simon Vetter
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

#include "string_encoder.h"

#include "src/db/database.h"
#include "src/main/helpers.h"
#include "src/main/item_types_handler.h"



QString StringEncoder::encodeInt(const QString& paramName, int value)
{
	return paramName + "=" + QString::number(value);
}

QString StringEncoder::encodeID(const QString& paramName, const ItemID& value)
{
	return paramName + "=" + (value.isInvalid() ? "-1" : value.asQVariant().toString());
}

QString StringEncoder::encodeBool(const QString& paramName, bool value)
{
	return paramName + "=" + (value ? "true" : "false");
}

QString StringEncoder::encodeString(const QString& paramName, const QString& value)
{
	return paramName + "=\"" + value.toHtmlEscaped() + "\"";
}

QString StringEncoder::encodeDate(const QString& paramName, const QDate& value)
{
	return paramName + "=" + value.toString(Qt::ISODate);
}

QString StringEncoder::encodeTime(const QString& paramName, const QTime& value)
{
	return paramName + "=" + value.toString(Qt::ISODate);
}



template<typename T>
T StringEncoder::decodeHeader(QString& restOfEncoding, const QString& encodedTypeSuffix, std::function<T(const QString&)> decodeType, bool& ok)
{
	auto fail = [&]() { ok = false; return T(-1); };
	
	const QString delimiter = encodedTypeSuffix + "(";
	const int typeNameLength = restOfEncoding.indexOf(delimiter);
	if (typeNameLength < 0) return fail();
	const QString columnTypeString = restOfEncoding.first(typeNameLength);
	const T type = decodeType(columnTypeString);
	restOfEncoding.remove(0, typeNameLength + delimiter.size());
	
	ok = true;
	return type;
}
template CompColType	StringEncoder::decodeHeader<CompColType>(QString&, const QString&, std::function<CompColType(const QString&)>, bool&);
template DataType		StringEncoder::decodeHeader<DataType>	(QString&, const QString&, std::function<DataType	(const QString&)>, bool&);

const NormalTable* StringEncoder::decodeTableIdentity(QString& restOfEncoding, const QString& tableNameParamName, const Database& db, bool& ok)
{
	const QString tableName = decodeString(restOfEncoding, tableNameParamName, ok);
	if (!ok) return nullptr;
	
	const QList<NormalTable*> normalTables = db.getNormalItemTableList();
	for (const NormalTable* const table : normalTables) {
		if (table->name == tableName) {
			return table;
		}
	}
	ok = false;
	return nullptr;
}

const Column* StringEncoder::decodeColumnIdentity(QString& restOfEncoding, const QString& tableNameParamName, const QString& columnNameParamName, const Database& db, bool& ok)
{
	const NormalTable* table = decodeTableIdentity(restOfEncoding, tableNameParamName, db, ok);
	if (!ok) return nullptr;
	
	const QString columnName = decodeString(restOfEncoding, columnNameParamName, ok);
	if (!ok) return nullptr;
	
	const QList<const Column*> columns = table->getColumnList();
	for (const Column* const column : columns) {
		if (column->name == columnName) {
			return column;
		}
	}
	ok = false;
	return nullptr;
}

const CompositeTable* StringEncoder::decodeCompTableIdentity(QString& restOfEncoding, const QString& tableNameParamName, const ItemTypesHandler& typesHandler, bool& ok)
{
	const QString tableName = decodeString(restOfEncoding, tableNameParamName, ok);
	if (!ok) return nullptr;
	
	for (const ItemTypeMapper* mapper : typesHandler.getAllMappers()) {
		if (mapper->compTable.name == tableName) {
			return &mapper->compTable;
		}
	}
	ok = false;
	return nullptr;
}

const CompositeColumn* StringEncoder::decodeCompColumnIdentity(QString& restOfEncoding, const QString& tableNameParamName, const QString& columnNameParamName, const ItemTypesHandler& typesHandler, bool& ok)
{
	const CompositeTable* table = decodeCompTableIdentity(restOfEncoding, tableNameParamName, typesHandler, ok);
	if (!ok) return nullptr;
	
	const QString columnName = decodeString(restOfEncoding, columnNameParamName, ok);
	if (!ok) return nullptr;
	
	const CompositeColumn* column = table->getColumnByNameOrNull(columnName);
	ok = column;
	return column;
}

int StringEncoder::decodeInt(QString& restOfEncoding, const QString& paramName, bool& ok)
{
	auto fail = [&]() { ok = false; return -1; };
	
	const QString expectedStart = paramName + "=";
	if (!restOfEncoding.startsWith(expectedStart)) return fail();
	restOfEncoding.remove(0, expectedStart.size());
	
	const QString endDelimiter = ",";
	const int valueLength = restOfEncoding.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfEncoding.first(valueLength);
	const int result = valueString.toInt(&ok);
	if (!ok) return fail();
	restOfEncoding.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}

ItemID StringEncoder::decodeID(QString& restOfEncoding, const QString& paramName, bool& ok)
{
	const int parsedInt = decodeInt(restOfEncoding, paramName, ok);
	
	if (!ok) return ItemID(-1);
	return ItemID(parsedInt);
}

bool StringEncoder::decodeBool(QString& restOfEncoding, const QString& paramName, bool& ok)
{
	auto fail = [&]() { ok = false; return false; };
	
	const QString expectedStart = paramName + "=";
	if (!restOfEncoding.startsWith(expectedStart)) return fail();
	restOfEncoding.remove(0, expectedStart.size());
	
	const QString endDelimiter = ",";
	const int valueLength = restOfEncoding.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfEncoding.first(valueLength);
	if (valueString != "true" && valueString != "false") return fail();
	const bool result = valueString == "true";
	restOfEncoding.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}

QString StringEncoder::decodeString(QString& restOfEncoding, const QString& paramName, bool& ok)
{
	auto fail = [&]() { ok = false; return QString(); };
	
	const QString expectedStart = paramName + "=\"";
	if (!restOfEncoding.startsWith(expectedStart)) return fail();
	restOfEncoding.remove(0, expectedStart.size());
	
	const QString endDelimiter = "\",";
	const int valueLength = restOfEncoding.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfEncoding.first(valueLength);
	const QString name = fromHtmlEscaped(valueString);
	restOfEncoding.remove(0, valueString.size() + endDelimiter.length());
	
	ok = true;
	return name;
}

QDate StringEncoder::decodeDate(QString& restOfEncoding, const QString& paramName, bool& ok)
{
	auto fail = [&]() { ok = false; return QDate(); };
	
	const QString expectedStart = paramName + "=";
	if (!restOfEncoding.startsWith(expectedStart)) return fail();
	restOfEncoding.remove(0, expectedStart.size());
	
	const QString endDelimiter = ",";
	const int valueLength = restOfEncoding.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfEncoding.first(valueLength);
	const QDate result = QDate::fromString(valueString, Qt::ISODate);
	if (!result.isValid()) return fail();
	restOfEncoding.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}

QTime StringEncoder::decodeTime(QString& restOfEncoding, const QString& paramName, bool& ok)
{
	auto fail = [&]() { ok = false; return QTime(); };
	
	const QString expectedStart = paramName + "=";
	if (!restOfEncoding.startsWith(expectedStart)) return fail();
	restOfEncoding.remove(0, expectedStart.size());
	
	const QString endDelimiter = ",";
	const int valueLength = restOfEncoding.indexOf(endDelimiter);
	if (valueLength < 0) return fail();
	const QString valueString = restOfEncoding.first(valueLength);
	const QTime result = QTime::fromString(valueString, Qt::ISODate);
	if (!result.isValid()) return fail();
	restOfEncoding.remove(0, valueLength + endDelimiter.length());
	
	ok = true;
	return result;
}
