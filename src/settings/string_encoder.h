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

#ifndef STRING_ENCODER_H
#define STRING_ENCODER_H

#include "src/data/item_id.h"
#include "src/db/normal_table.h"

#include <QString>

class CompositeColumn;
class CompositeTable;
class ItemTypesHandler;



class StringEncoder
{
public:
	// Encoding helpers
	static QString encodeInt	(const QString& paramName, int value);
	static QString encodeID		(const QString& paramName, const ItemID& value);
	static QString encodeBool	(const QString& paramName, bool value);
	static QString encodeString	(const QString& paramName, const QString& value);
	static QString encodeDate	(const QString& paramName, const QDate& value);
	static QString encodeTime	(const QString& paramName, const QTime& value);
	
	// Decoding helpers
	template<typename T>
	static T						decodeHeader			(QString& restOfEncoding, const QString& encodedTypeSuffix, std::function<T(const QString&)> decodeType, bool& ok);
	static const NormalTable*		decodeTableIdentity		(QString& restOfEncoding, const QString& tableNameParamName, const Database& db, bool& ok);
	static const Column*			decodeColumnIdentity	(QString& restOfEncoding, const QString& tableNameParamName, const QString& columnNameParamName, const Database& db, bool& ok);
	static const CompositeTable*	decodeCompTableIdentity	(QString& restOfEncoding, const QString& tableNameParamName, const ItemTypesHandler& typesHandler, bool& ok);
	static const CompositeColumn*	decodeCompColumnIdentity(QString& restOfEncoding, const QString& tableNameParamName, const QString& columnNameParamName, const ItemTypesHandler& typesHandler, bool& ok);
	static int						decodeInt				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static ItemID					decodeID				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static bool						decodeBool				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static QString					decodeString			(QString& restOfEncoding, const QString& paramName, bool& ok);
	static QDate					decodeDate				(QString& restOfEncoding, const QString& paramName, bool& ok);
	static QTime					decodeTime				(QString& restOfEncoding, const QString& paramName, bool& ok);
};



#endif // STRING_ENCODER_H
