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

#ifndef COMP_COLUMN_TYPE_H
#define COMP_COLUMN_TYPE_H

#include <QStringList>



enum CompColType {
	Direct,
	Reference,
	Difference,
	DependentEnum,
	Index,
	Ordinal,
	CountFold,
	NumericFold,
	ListStringFold,
	HikerListFold
};



/**
 * A struct containing the names of the composite column types and providing methods to convert
 * between names and the CompColType enum.
 */
struct CompColTypeNames
{
	/** A list of the names of the composite column types, in the order of the CompColType enum. */
	inline static const QStringList compColTypeNames = {
		"Direct", "Reference", "Difference", "DependentEnum", "Index", "Ordinal", "CountFold", "NumericFold", "ListStringFold", "HikerListFold"
	};
	
	/**
	 * Returns the name of the given composite column type or an empty string if the type is
	 * invalid.
	 * 
	 * @param type	The composite column type.
	 */
	inline static QString getName(const CompColType type)
	{
		if (type < 0 || type >= compColTypeNames.size()) return QString();
		return compColTypeNames.at(type);
	}
	
	/**
	 * Returns the composite column type corresponding to the given name or -1 if the name is
	 * invalid.
	 * 
	 * @param name	The name of the composite column type.
	 */
	inline static CompColType getType(const QString& name)
	{
		if (!compColTypeNames.contains(name)) return CompColType(-1);
		return CompColType(compColTypeNames.indexOf(name));
	}
};



#endif // COMP_COLUMN_TYPE_H
