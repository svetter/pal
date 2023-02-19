#include "ranges_table.h"

#include <QString>
#include <QTranslator>



RangesTable::RangesTable() :
		NormalTable(QString("Ranges"), tr("Mountain ranges"), "rangeID"),
		//										name			uiName				type		nullable	primaryKey	foreignKey	inTable
		nameColumn		(new const Column(QString("name"),		tr("Name"),			varchar,	false,		false,		nullptr,	this)),
		continentColumn	(new const Column(QString("continent"),	tr("Continent"),	integer,	false,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(continentColumn);
}



int RangesTable::addRow(QWidget* parent, const Range* range)
{
	assert(range->rangeID == -1);
	QList<QVariant> data = mapDataToQVariantList(range);
	
	int newRangeIndex = NormalTable::addRow(parent, data);
	return newRangeIndex;
}

void RangesTable::updateRow(QWidget* parent, ValidItemID hikerID, const Range* range)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(range);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> RangesTable::mapDataToQVariantList(const Range* range) const
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
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
