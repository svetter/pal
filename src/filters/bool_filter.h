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

#ifndef BOOL_FILTER_H
#define BOOL_FILTER_H

#include "filter.h"



class BoolFilter : public Filter
{
	bool value;
	
public:
	BoolFilter(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
	virtual ~BoolFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static BoolFilter* decodeTypeSpecific(const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName, QString& restOfEncoding);
	
	friend class Filter;
	friend class BoolFilterBox;
};



#endif // BOOL_FILTER_H
