/*
 * Copyright 2023 Simon Vetter
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
 * @file Filter.h
 * 
 * This file defines the Filter class.
 */

#ifndef FILTER_H
#define FILTER_H

#include <QVariant>
#include <QHashFunctions>

class CompositeColumn;



/**
 * A filter for a CompositeColumn, which can have one or two values used for filtering.
 */
class Filter {
public:
	/** The column to use for filtering rows. */
	const CompositeColumn* column;
	/** The first (and possibly only) value to use for filtering rows. */
	const QVariant value;
	/** Whether a second value is set for filtering rows. */
	const bool hasSecond;
	/** The second value to use for filtering rows. */
	const QVariant secondValue;
	
	/**
	 * Creates a new filter with one value.
	 * 
	 * @param column		The column to use for filtering rows.
	 * @param value			The value to use for filtering rows.
	 */
	inline Filter(const CompositeColumn* column, QVariant value) :
			column(column),
			value(value),
			hasSecond(false),
			secondValue(QVariant())
	{}

	/**
	 * Creates a new filter with two values.
	 *
	 * @param column		The column to use for filtering rows.
	 * @param value			The first value to use for filtering rows.
	 * @param secondValue	The second value to use for filtering rows.
	 */
	inline Filter(const CompositeColumn* column, QVariant value, QVariant secondValue) :
			column(column),
			value(value),
			hasSecond(true),
			secondValue(secondValue)
	{}
};



/**
 * Compares two filters for equality.
 * 
 * @param filter1	The first filter to compare.
 * @param filter2	The second filter to compare.
 * @return			True if the filters are equal, false otherwise.
 */
inline bool operator==(const Filter& filter1, const Filter& filter2)
{
	if (filter1.column != filter2.column) return false;
	if (filter1.value != filter2.value) return false;
	if (filter1.hasSecond != filter2.hasSecond) return false;
	if (!filter1.hasSecond && !filter2.hasSecond) return true;
	return filter1.secondValue == filter2.secondValue;
}

/**
 * Compares two filters for inequality.
 * 
 * @param filter1	The first filter to compare.
 * @param filter2	The second filter to compare.
 * @return			True if the filters are not equal, false otherwise.
 */
inline bool operator!=(const Filter& filter1, const Filter& filter2)
{
	return !operator==(filter1, filter2);
}

/**
 * Hashes the given filter using the given seed.
 * 
 * This implementation is required for using Filter in QSets and implicitly comparing instances of
 * QList<Filter>.
 * 
 * @param key	The filter to hash.
 * @param seed	The seed to use for hashing.
 * @return		The hash value.
 */
inline size_t qHash(const Filter& key, size_t seed)
{
	return qHashMulti(seed, (void*) key.column, key.value.toByteArray(), key.hasSecond, key.secondValue.toByteArray());
}



#endif // FILTER_H
