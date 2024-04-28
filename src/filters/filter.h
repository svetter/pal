#ifndef FILTER_H
#define FILTER_H

#include "src/comp_tables/composite_table.h"
#include "src/settings/string_encoder.h"

class FilterBox;



class Filter : public StringEncoder
{
public:
	const DataType type;
	const CompositeTable& tableToFilter;
	const CompositeColumn& columnToFilterBy;
public:
	const QString uiName;
	
private:
	bool enabled;
	bool inverted;
	
protected:
	Filter(DataType type, const CompositeTable& tableToFilter, const CompositeColumn& columnToFilterBy, const QString& uiName);
public:
	virtual ~Filter();
	
public:
	bool isEnabled() const;
	bool isInverted() const;
	void setEnabled(bool enabled);
	void setInverted(bool inverted);
	
	void applyToOrderBuffer(ViewOrderBuffer& viewOrderBuffer) const;
protected:
	virtual bool evaluate(const QVariant& rawRowValue) const = 0;
	
public:
	virtual FilterBox* createFilterBox(QWidget* parent) = 0;
	
	
	// Encoding & Decoding
	static QString encodeToString(QList<const Filter*> filters);
private:
	QString encodeSingleFilterToString() const;
public:
	static QList<Filter*> decodeFromString(const QString& encoded, const ItemTypesHandler& typesHandler);
private:
	static Filter* decodeSingleFilterFromString(QString& restOfEncoding, const ItemTypesHandler& typesHandler);
protected:
	virtual QStringList encodeTypeSpecific() const = 0;
};



#endif // FILTER_H
