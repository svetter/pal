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

/**
 * @file row_index.cpp
 * 
 * This file defines the RowIndex class and its subclasses.
 */

#include "row_index.h"

#include "qhashfunctions.h"



/**
 * Creates a new RowIndex from an int.
 * 
 * @param index	The index.
 */
RowIndex::RowIndex(int index) : index(index)
{
	set(index);
}


/**
 * Sets a new value for the index.
 * 
 * @param newIndex	The new index. Values below zero denote an invalid index.
 */
void RowIndex::set(int newIndex)
{
	index = newIndex;
}


/**
 * Returns whether the index is below zero.
 * 
 * @return	True if the index is below zero, false otherwise.
 */
bool RowIndex::isBelowZero() const
{
	return index < 0;
}

/**
 * Returns whether the index is greater than or equal to the given number of rows, making it
 * invalid.
 * 
 * @param numberOfRows	The number of rows to compare the index to.
 * @return				True if the index is too high to be valid, false otherwise.
 */
bool RowIndex::isAboveRange(int numberOfRows) const
{
	assert(numberOfRows >= 0);
	return index >= numberOfRows;
}

/**
 * Returns whether the index is invalid.
 * 
 * An index is invalid if it is below zero or greater than or equal to the optional number of rows.
 * 
 * @param numberOfRows	The number of rows to compare the index to. Ignored if negative.
 * @return				True if the index is invalid, false otherwise.
 */
bool RowIndex::isInvalid(int numberOfRows) const
{
	return isBelowZero() || (numberOfRows >= 0 && isAboveRange(numberOfRows));
}

/**
 * Returns whether the index is valid.
 * 
 * An index is valid if it is greater than or equal to zero and less than the optional number of
 * rows.
 * 
 * @param numberOfRows	The number of rows to compare the index to. Ignored if negative.
 * @return				True if the index is valid, false otherwise.
 */
bool RowIndex::isValid(int numberOfRows) const
{
	return !isInvalid(numberOfRows);
}


/**
 * Asserts that the index is valid.
 * 
 * @param numberOfRows	The number of rows to compare the index to. Ignored if negative.
 */
void RowIndex::assertValid(int numberOfRows) const
{
	assert(isValid(numberOfRows));
}


/**
 * Returns the index as it is stored internally.
 * 
 * @return	The raw index value.
 */
int RowIndex::getRaw() const
{
	return index;
}

/**
 * Returns the index.
 * 
 * Negative values are converted to -1.
 * 
 * @return	The index.
 */
int RowIndex::get() const
{
	return index < 0 ? -1 : index;
}


/**
 * Returns whether this index is equal to the given index.
 * 
 * @param other	The index to compare to.
 * @return		True if the indices are equal, false otherwise.
 */
bool RowIndex::operator==(const RowIndex& other) const
{
	return get() == other.get();
}


/**
 * Returns whether this index is less than the given index.
 * 
 * @pre Both indices are valid.
 * 
 * @param compIndex	The index to compare to.
 * @return			True if this index is less than the given index, false otherwise.
 */
bool RowIndex::operator<(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index < compIndex.get();
}

/**
 * Returns whether this index is less than or equal to the given index.
 * 
 * @pre Both indices are valid.
 * 
 * @param compIndex	The index to compare to.
 * @return			True if this index is less than or equal to the given index, false otherwise.
 */
bool RowIndex::operator<=(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index <= compIndex.get();
}

/**
 * Returns whether this index is greater than the given index.
 * 
 * @pre Both indices are valid.
 * 
 * @param compIndex	The index to compare to.
 * @return			True if this index is greater than the given index, false otherwise.
 */
bool RowIndex::operator>(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index > compIndex.get();
}

/**
 * Returns whether this index is greater than or equal to the given index.
 * 
 * @pre Both indices are valid.
 * 
 * @param compIndex	The index to compare to.
 * @return			True if this index is greater than or equal to the given index, false otherwise.
 */
bool RowIndex::operator>=(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index >= compIndex.get();
}


/**
 * Increments the index.
 * 
 * @pre The index is valid.
 */
void RowIndex::operator++(int)
{
	assertValid();
	set(get() + 1);
}

/**
 * Decrements the index.
 *
 * @pre The index is valid.
 */
void RowIndex::operator--(int)
{
	assertValid();
	set(get() - 1);
}



/**
 * Hashes the given index using the given seed.
 *
 * This implementation is required for using RowIndex in QSets and implicitly comparing instances
 * of QList<ItemID>.
 *
 * @param index	The index to hash.
 * @param seed	The seed to use for hashing.
 * @return		The hash value.
 */
size_t qHash(const RowIndex& index, size_t seed)
{
	return qHashMulti(seed, index.get());
}





/**
 * Creates an invalid BufferRowIndex.
 */
BufferRowIndex::BufferRowIndex() : RowIndex(-1)
{}

/**
 * Creates a new BufferRowIndex from an int.
 * 
 * @param index	The index.
 */
BufferRowIndex::BufferRowIndex(int index) : RowIndex(index)
{}

/**
 * Creates a new BufferRowIndex from another BufferRowIndex.
 * 
 * @param other	The BufferRowIndex to copy.
 */
BufferRowIndex::BufferRowIndex(const BufferRowIndex& other) : RowIndex(other.getRaw())
{}


/**
 * Assigns a new value to this index from another BufferRowIndex.
 * 
 * @param newIndex	The new index.
 * @return			A reference to this BufferRowIndex.
 */
BufferRowIndex& BufferRowIndex::operator=(const BufferRowIndex& newIndex)
{
	set(newIndex.getRaw());
	return *this;
}


/**
 * Returns the sum of this index and the given summand as a new BufferRowIndex.
 * 
 * @pre The index is valid.
 * 
 * @param summand	The summand.
 * @return			The sum.
 */
BufferRowIndex BufferRowIndex::operator+(int summand) const
{
	assertValid();
	return BufferRowIndex(get() + summand);
}

/**
 * Returns the difference of this index and the given subtrahend as a new BufferRowIndex.
 * 
 * @pre The index is valid.
 * 
 * @param subtrahend	The subtrahend.
 * @return				The difference.
 */
BufferRowIndex BufferRowIndex::operator-(int subtrahend) const
{
	assertValid();
	return BufferRowIndex(get() - subtrahend);
}


/**
 * Compares a BufferRowIndex to an int for equality.
 * 
 * @param index1	The BufferRowIndex.
 * @param index2	The int.
 * @return			True if the indices are equal or both invalid, false otherwise.
 */
bool operator==(const BufferRowIndex& index1, int index2)
{
	return index1 == BufferRowIndex(index2);
}

/**
 * Compares an int to a BufferRowIndex for equality.
 * 
 * @param index1	The int.
 * @param index2	The BufferRowIndex.
 * @return			True if the indices are equal or both invalid, false otherwise.
 */
bool operator==(int index1, const BufferRowIndex& index2)
{
	return BufferRowIndex(index1) == index2;
}



/**
 * Hashes the given BufferRowIndex.
 * 
 * This implementation is required for using BufferRowIndex as keys in a QHash.
 * 
 * @param key	The BufferRowIndex to hash.
 * @return		The hash value.
 */
size_t qHash(const BufferRowIndex& key)
{
	return qHashMulti(key.isValid(), key.isValid() ? key.get() : 0);
}





/**
 * Creates an invalid ViewRowIndex.
 */
ViewRowIndex::ViewRowIndex() : RowIndex(-1)
{}

/**
 * Creates a new ViewRowIndex from an int.
 * 
 * @param index	The index.
 */
ViewRowIndex::ViewRowIndex(int index) : RowIndex(index)
{}

/**
 * Creates a new ViewRowIndex from another ViewRowIndex.
 * 
 * @param other	The ViewRowIndex to copy.
 */
ViewRowIndex::ViewRowIndex(const ViewRowIndex& other) : RowIndex(other.getRaw())
{}


/**
 * Assigns a new value to this index from another ViewRowIndex.
 * 
 * @param newIndex	The new index.
 * @return			A reference to this ViewRowIndex.
 */
ViewRowIndex& ViewRowIndex::operator=(const ViewRowIndex& newIndex)
{
	set(newIndex.getRaw());
	return *this;
}


/**
 * Returns the sum of this index and the given summand as a new ViewRowIndex.
 * 
 * @pre The index is valid.
 * 
 * @param summand	The summand.
 * @return			The sum.
 */
ViewRowIndex ViewRowIndex::operator+(int summand) const
{
	assertValid();
	return ViewRowIndex(get() + summand);
}

/**
 * Returns the difference of this index and the given subtrahend as a new ViewRowIndex.
 * 
 * @pre The index is valid.
 * 
 * @param subtrahend	The subtrahend.
 * @return				The difference.
 */
ViewRowIndex ViewRowIndex::operator-(int subtrahend) const
{
	assertValid();
	return ViewRowIndex(get() - subtrahend);
}


/**
 * Compares a ViewRowIndex to an int for equality.
 * 
 * @param index1	The ViewRowIndex.
 * @param index2	The int.
 * @return			True if the indices are equal or both invalid, false otherwise.
 */
bool operator==(const ViewRowIndex& index1, int index2)
{
	return index1 == ViewRowIndex(index2);
}

/**
 * Compares an int to a ViewRowIndex for equality.
 * 
 * @param index1	The int.
 * @param index2	The ViewRowIndex.
 * @return			True if the indices are equal or both invalid, false otherwise.
 */
bool operator==(int index1, const ViewRowIndex& index2)
{
	return ViewRowIndex(index1) == index2;
}
