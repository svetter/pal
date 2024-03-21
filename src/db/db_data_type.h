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
 * @file db_data_type.h
 * 
 * This file defines the DataType enum and the DataTypeNames struct.
 */

#ifndef DB_DATA_TYPE_H
#define DB_DATA_TYPE_H

#include <QList>



/**
 * Specifies the type of data contained in a database column.
 * 
 * Items correspond to SQL data types, except for the ID type.
 */
enum DataType {
	Integer, ID, Enum, DualEnum, Bit, String, Date, Time, IDList
};



/**
 * A struct containing the names of the data types and providing methods to convert between names
 * and the DataType enum.
 */
struct DataTypeNames
{
	/** A list of the names of the data types, in the order of the DataType enum. */
	inline static const QList<QString> dataTypeNames = {
		"Integer", "ID", "Enum", "DualEnum", "Bit", "String", "Date", "Time", "IDList"
	};
	
	/**
	 * Returns the name of the given data type or an empty string if the type is invalid.
	 * 
	 * @param type	The data type.
	 */
	inline static QString getName(DataType type)
	{
		if (type < 0 || type >= dataTypeNames.size()) return QString();
		return dataTypeNames.at(type);
	}
	
	/**
	 * Returns the data type corresponding to the given name or -1 if the name is invalid.
	 * 
	 * @param name	The name of the data type.
	 */
	inline static DataType getType(const QString& name)
	{
		if (!dataTypeNames.contains(name)) return DataType(-1);
		return DataType(dataTypeNames.indexOf(name));
	}
};



#endif // DB_DATA_TYPE_H
