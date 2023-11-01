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

#include "row_index.h"

#include "qhashfunctions.h"



RowIndex::RowIndex(int index) : index(index)
{
	set(index);
}


void RowIndex::set(int newIndex)
{
	index = newIndex;
}


bool RowIndex::isBelowZero() const
{
	return index < 0;
}

bool RowIndex::isAboveRange(int numberOfRows) const
{
	assert(numberOfRows >= 0);
	return index >= numberOfRows;
}

bool RowIndex::isInvalid(int numberOfRows) const
{
	return isBelowZero() || (numberOfRows >= 0 && isAboveRange(numberOfRows));
}

bool RowIndex::isValid(int numberOfRows) const
{
	return !isInvalid(numberOfRows);
}


void RowIndex::assertValid(int numberOfRows) const
{
	assert(isValid(numberOfRows));
}


int RowIndex::getRaw() const
{
	return index;
}

int RowIndex::get() const
{
	return index < 0 ? -1 : index;
}


bool RowIndex::operator==(const RowIndex& other) const
{
	return get() == other.get();
}


bool RowIndex::operator<(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index < compIndex.get();
}
bool RowIndex::operator<=(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index <= compIndex.get();
}
bool RowIndex::operator>(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index > compIndex.get();
}
bool RowIndex::operator>=(RowIndex compIndex) const
{
	assertValid();
	compIndex.assertValid();
	return index >= compIndex.get();
}


void RowIndex::operator++(int)
{
	assertValid();
	set(get() + 1);
}

void RowIndex::operator--(int)
{
	assertValid();
	set(get() - 1);
}



size_t qHash(const RowIndex& index, size_t seed)
{
	return qHashMulti(seed, index.get());
}





BufferRowIndex::BufferRowIndex() : RowIndex(-1)
{}

BufferRowIndex::BufferRowIndex(int index) : RowIndex(index)
{}

BufferRowIndex::BufferRowIndex(const BufferRowIndex& other) : RowIndex(other.getRaw())
{}


BufferRowIndex& BufferRowIndex::operator=(const BufferRowIndex& newIndex)
{
	set(newIndex.getRaw());
	return *this;
}


BufferRowIndex BufferRowIndex::operator+(int summand) const
{
	assertValid();
	return BufferRowIndex(get() + summand);
}

BufferRowIndex BufferRowIndex::operator-(int subtrahend) const
{
	assertValid();
	return BufferRowIndex(get() - subtrahend);
}


bool operator==(const BufferRowIndex& index1, int index2)
{
	return index1 == BufferRowIndex(index2);
}

bool operator==(int index1, const BufferRowIndex& index2)
{
	return BufferRowIndex(index1) == index2;
}





ViewRowIndex::ViewRowIndex() : RowIndex(-1)
{}

ViewRowIndex::ViewRowIndex(int index) : RowIndex(index)
{}

ViewRowIndex::ViewRowIndex(const ViewRowIndex& other) : RowIndex(other.getRaw())
{}


ViewRowIndex& ViewRowIndex::operator=(const ViewRowIndex& newIndex)
{
	set(newIndex.getRaw());
	return *this;
}


ViewRowIndex ViewRowIndex::operator+(int summand) const
{
	assertValid();
	return ViewRowIndex(get() + summand);
}

ViewRowIndex ViewRowIndex::operator-(int subtrahend) const
{
	assertValid();
	return ViewRowIndex(get() - subtrahend);
}


bool operator==(const ViewRowIndex& index1, int index2)
{
	return index1 == ViewRowIndex(index2);
}

bool operator==(int index1, const ViewRowIndex& index2)
{
	return ViewRowIndex(index1) == index2;
}
