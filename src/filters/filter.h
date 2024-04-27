#ifndef FILTER_H
#define FILTER_H

#include "src/settings/string_encoder.h"
#include "src/db/breadcrumbs.h"
#include "src/db/normal_table.h"
#include "src/comp_tables/numeric_fold_op.h"

class FilterBox;



class Filter : public StringEncoder
{
public:
	const DataType type;
	const DataType sourceType;
	const NormalTable& tableToFilter;
	const Column& columnToFilterBy;
	const NumericFoldOp foldOp;
protected:
	const Breadcrumbs crumbs;
public:
	const bool isLocalFilter;
	const bool singleValuePerRow;
	
	const QString name;
	
private:
	bool enabled;
	bool inverted;
	
protected:
	Filter(DataType type, const NormalTable& tableToFilter, const Column& columnToFilterBy, NumericFoldOp foldOp, const QString& name);
public:
	virtual ~Filter();
	
public:
	bool isEnabled() const;
	bool isInverted() const;
	void setEnabled(bool enabled);
	void setInverted(bool inverted);
	
	void applyToOrderBuffer(ViewOrderBuffer& viewOrderBuffer) const;
private:
	QVariant getRawRowValue(const BufferRowIndex filteredTableBufferRow) const;
protected:
	virtual bool evaluate(const QVariant& rawRowValue) const = 0;
	
public:
	virtual FilterBox* createFilterBox(QWidget* parent) = 0;
	
	
	// Encoding & Decoding
	static QString encodeToString(QList<const Filter*> filters);
private:
	QString encodeSingleFilterToString() const;
public:
	static QList<Filter*> decodeFromString(const QString& encoded, Database& db);
private:
	static Filter* decodeSingleFilterFromString(QString& restOfEncoding, Database& db);
protected:
	virtual QStringList encodeTypeSpecific() const = 0;
};



#endif // FILTER_H
