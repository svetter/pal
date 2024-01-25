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
 * @file enum_names.h
 * 
 * This file declares the EnumNames class containing names for enumerative fields in items.
 */

#ifndef ENUM_NAMES_H
#define ENUM_NAMES_H

#include "qobject.h"



/**
 * A class containing name strings for enumerative fields in items.
 * 
 * The names are translated dynamically at runtime using translateList().
 */
class EnumNames : private QObject {
	Q_OBJECT
	
public:
	static QStringList translateList(const QStringList& list);
	
	static const QStringList continentNames;
	static const QStringList hikeKindNames;
	static const QList<QPair<QString, QStringList>> difficultyNames;
};



#endif // ENUM_NAMES_H
