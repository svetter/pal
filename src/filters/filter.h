#ifndef FILTER_H
#define FILTER_H

#include "src/db/normal_table.h"

class FilterBox;



class Filter
{
public:
	const DataType type;
	const NormalTable& tableToFilter;
	const Column& columnToFilterBy;
	
	const QString name;
	
private:
	bool inverted;
	
protected:
	Filter(DataType type, const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	
public:
	bool isInverted() const;
	void setInverted(bool inverted);
	
	virtual FilterBox* getFilterBox(QWidget* parent) const = 0;
};



#endif // FILTER_H
