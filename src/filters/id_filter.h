#ifndef ID_FILTER_H
#define ID_FILTER_H

#include "filter.h"



class IDFilter : public Filter
{
	ItemID value;
	
public:
	IDFilter(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name);
	virtual ~IDFilter();
	
	virtual bool evaluate(const QVariant& rawRowValue) const override;
	
	virtual FilterBox* createFilterBox(QWidget* parent) override;
	
protected:
	virtual QStringList encodeTypeSpecific() const override;
	static IDFilter* decodeTypeSpecific(const NormalTable& tableToFilter, const Column& columnToFilterBy, const QString& name, QString& restOfEncoding);
	
	friend class Filter;
	friend class IDFilterBox;
};



#endif // ID_FILTER_H
