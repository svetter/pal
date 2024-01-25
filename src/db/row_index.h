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
 * @file row_index.h
 * 
 * This file declares the RowIndex class and its subclasses.
 */

#ifndef ROW_INDEX_H
#define ROW_INDEX_H

#include <stddef.h>



/**
 * An int wrapper class to represent a row index in either a buffer or a view.
 * 
 * The subclasses BufferRowIndex and ViewRowIndex are deliberately incompatible to prevent
 * accidental casting or otherwise conflating the two.
 */
class RowIndex {
	/** The index. Values below zero denote an invalid index. */
	int index;
	
protected:
	explicit RowIndex(int index);
	
	void set(int newIndex);
	
public:
	bool isBelowZero() const;
	bool isAboveRange(int numberOfRows) const;
	bool isInvalid(int numberOfRows = -1) const;
	bool isValid(int numberOfRows = -1) const;
	
protected:
	void assertValid(int numberOfRows = -1) const;
	
	int getRaw() const;
public:
	int get() const;
	
	bool operator==(const RowIndex& other) const;
	
	bool operator<(RowIndex compIndex) const;
	bool operator<=(RowIndex compIndex) const;
	bool operator>(RowIndex compIndex) const;
	bool operator>=(RowIndex compIndex) const;
	
	bool operator< (int compIndex) const = delete;
	bool operator<=(int compIndex) const = delete;
	bool operator> (int compIndex) const = delete;
	bool operator>=(int compIndex) const = delete;
	
	void operator++(int);
	void operator--(int);
};



size_t qHash(const RowIndex& index, size_t seed);





/**
 * An int wrapper class to represent a row index in a buffer.
 */
class BufferRowIndex : public RowIndex {
public:
	BufferRowIndex();
	explicit BufferRowIndex(int index);
	BufferRowIndex(const BufferRowIndex& other);
	
	BufferRowIndex& operator=(const BufferRowIndex& newIndex);
	
	BufferRowIndex operator+(int summand) const;
	BufferRowIndex operator-(int subtrahend) const;
};

bool operator==(const BufferRowIndex& index1, int index2);
bool operator==(int index1, const BufferRowIndex& index2);





/**
 * An int wrapper class to represent a row index in a view.
 */
class ViewRowIndex : public RowIndex {
public:
	ViewRowIndex();
	explicit ViewRowIndex(int index);
	ViewRowIndex(const ViewRowIndex& other);
	
	ViewRowIndex& operator=(const ViewRowIndex& newIndex);
	
	ViewRowIndex operator+(int summand) const;
	ViewRowIndex operator-(int subtrahend) const;
};

bool operator==(const ViewRowIndex& index1, int index2);
bool operator==(int index1, const ViewRowIndex& index2);



#endif // ROW_INDEX_H
