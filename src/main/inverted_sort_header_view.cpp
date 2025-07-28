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
 * @file inverted_sort_header_view.cpp
 * 
 * This file defines the InvertedSortHeaderView class.
 */

#include "inverted_sort_header_view.h"

#include "src/settings/settings.h"

#include <QMouseEvent>



InvertedSortHeaderView::InvertedSortHeaderView(QWidget* parent, const CompositeTable& table) :
	QHeaderView(Qt::Orientation::Horizontal, parent),
	table(table)
{}

InvertedSortHeaderView::~InvertedSortHeaderView()
{}



void InvertedSortHeaderView::mousePressEvent(QMouseEvent* event)
{
	const int logicalIndex = logicalIndexAt(event->pos());
	
	const CompositeColumn& column = table.getColumnAt(logicalIndex);
	const bool isNumericColumn = column.contentType == Integer || column.contentType == Date || column.contentType == Time;
	
	const bool applyDefaultOrder = logicalIndex != sortIndicatorSection();
	const bool invertDefaultOrder = isNumericColumn && Settings::sortNumericColumnsDescendingByDefault.get();
	const bool applyInvertedDefaultOrder = applyDefaultOrder && invertDefaultOrder;
	
	if (applyInvertedDefaultOrder) {
		// Default to decending
		const bool oldBlockState = this->blockSignals(true);
		setSortIndicator(logicalIndex, Qt::AscendingOrder);
		this->blockSignals(oldBlockState);
	}
	
	QHeaderView::mousePressEvent(event);
}
