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
 * @file breadcrumbs.h
 * 
 * This file declares the Breadcrumb struct and the Breadcrumbs class.
 */

#ifndef BREADCRUMBS_H
#define BREADCRUMBS_H

#include "src/db/normal_table.h"



/**
 * A class representing one pair of columns in a chain of "breadcrumbs" to be followed in order to
 * collect a set of items connected to a starting item.
 * 
 * @see Breadcrumbs
 */
class Breadcrumb {
public:
	/** The first column in the breadcrumb pair. */
	Column& firstColumn;
	/** The second column in the breadcrumb pair. */
	Column& secondColumn;
	
protected:
	Breadcrumb(Column& firstColumn, Column& secondColumn);
public:
	Breadcrumb(ForeignKeyColumn& firstColumn, PrimaryKeyColumn& secondColumn);
	Breadcrumb(PrimaryKeyColumn& firstColumn, ForeignKeyColumn& secondColumn);
	
	bool isForward() const;
	bool isBackward() const;
	
	bool operator==(const Breadcrumb& other) const;
	bool operator!=(const Breadcrumb& other) const;
};



/**
 * A class representing a chain of "breadcrumbs" to be followed in order to collect a set of items
 * connected to a starting item.
 * 
 * @see evaluate()
 */
class Breadcrumbs
{
	QList<Breadcrumb> list;
	
public:
	Breadcrumbs();
	Breadcrumbs(std::initializer_list<Breadcrumb> initList);
	Breadcrumbs(const QList<Breadcrumb>& initList);
	
	const QSet<const Column*> getColumnSet() const;
	const NormalTable& getStartTable() const;
	const NormalTable& getTargetTable() const;
	bool isEmpty() const;
	int length() const;
	bool isForwardOnly() const;
	bool goesVia(const Table& table) const;
	
	bool operator==(const Breadcrumbs& other) const;
	bool operator!=(const Breadcrumbs& other) const;
	
	void append(const Breadcrumb& breadcrumb);
	
	Breadcrumbs operator+(const Breadcrumbs& other) const;
	
	QSet<BufferRowIndex> evaluate(BufferRowIndex initialBufferRowIndex) const;
	BufferRowIndex evaluateAsForwardChain(BufferRowIndex initialBufferRowIndex) const;
	QList<BufferRowIndex> evaluateForStats(const QSet<BufferRowIndex>& initialBufferRowIndices) const;
};



#endif // BREADCRUMBS_H
