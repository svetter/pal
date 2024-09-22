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
 * @file inverted_sort_header_view.h
 * 
 * This file declares the InvertedSortHeaderView class.
 */

#ifndef INVERTED_SORT_HEADER_VIEW_H
#define INVERTED_SORT_HEADER_VIEW_H

#include "src/comp_tables/composite_table.h"

#include <QHeaderView>



class InvertedSortHeaderView : public QHeaderView
{
	Q_OBJECT
	
	const CompositeTable& table;
	
public:
	InvertedSortHeaderView(QWidget* parent, const CompositeTable& table);
	virtual ~InvertedSortHeaderView();
	
protected:
	void mousePressEvent(QMouseEvent* event) override;
};



#endif // INVERTED_SORT_HEADER_VIEW_H
