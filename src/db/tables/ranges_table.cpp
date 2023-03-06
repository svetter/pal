#include "ranges_table.h"

#include <QString>
#include <QTranslator>



RangesTable::RangesTable() :
		NormalTable(QString("Ranges"), tr("Mountain ranges"), "rangeID"),
		//							name			uiName				type		nullable	primaryKey	foreignKey	inTable
		nameColumn		(new Column("name",			tr("Name"),			varchar,	false,		false,		nullptr,	this)),
		continentColumn	(new Column("continent",	tr("Continent"),	integer,	false,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
	addColumn(continentColumn);
}



int RangesTable::addRow(QWidget* parent, Range* range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, range);
	
	int newRangeIndex = NormalTable::addRow(parent, columns, data);
	range->rangeID = getPrimaryKeyAt(newRangeIndex);
	return newRangeIndex;
}

void RangesTable::updateRow(QWidget* parent, ValidItemID hikerID, const Range* range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, range);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> RangesTable::mapDataToQVariantList(QList<const Column*>& columns, const Range* range) const
{
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)		{ data.append(range->name);			continue; }
		if (column == continentColumn)	{ data.append(range->continent);	continue; }
		assert(false);
	}
	return data;
}



QString RangesTable::getNoneString() const
{
	return tr("None");
}

QString RangesTable::getItemNameSingularLowercase() const
{
	return tr("mountain range");
}

QString RangesTable::getItemNamePluralLowercase() const
{
	return tr("mountain ranges");
}
