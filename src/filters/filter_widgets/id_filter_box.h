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

#ifndef ID_FILTER_BOX_H
#define ID_FILTER_BOX_H

#include "src/data/item_id.h"
#include "src/filters/filter_widgets/filter_box.h"
#include "src/filters/id_filter.h"

#include <QComboBox>



class IDFilterBox : public FilterBox
{
	Q_OBJECT
	
	IDFilter& filter;
	
	QComboBox* combo;
	std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo;
	
	QList<ValidItemID> selectableItemIDs;
	
public:
	explicit IDFilterBox(QWidget* parent, const QString& title, std::function<void (QComboBox&, QList<ValidItemID>&)> populateItemCombo, IDFilter& filter);
	virtual ~IDFilterBox();
	
	void setComboEntries();
	
	virtual void updateFilterTypeSpecific();
	
	virtual const Filter& getFilter() const;
};



#endif // ID_FILTER_BOX_H
