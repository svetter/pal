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

#ifndef ID_FILTER_H
#define ID_FILTER_H

#include "filter.h"



class IDFilter : public Filter
{
	ItemID value;
	
public:
	IDFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
	virtual ~IDFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static IDFilter* decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding);
	
	friend class Filter;
	friend class IDFilterBox;
};



#endif // ID_FILTER_H
