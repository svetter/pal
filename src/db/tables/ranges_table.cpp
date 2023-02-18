#include "ranges_table.h"

#include <QString>
#include <QTranslator>



RangesTable::RangesTable() :
		NormalTable(QString("Ranges"), tr("Mountain ranges"), "rangeID"),
		//										name			uiName				type		nullable	primaryKey	foreignKey	inTable
		nameColumn		(new const Column(QString("name"),		tr("Name"),			varchar,	false,		false,		nullptr,	this)),
		continentColumn	(new const Column(QString("continent"),	tr("Continent"),	integer,	true,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(continentColumn);
}



int RangesTable::addRow(QWidget* parent, const Range* range)
{
	assert(range->rangeID == -1);
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == nameColumn)		{ data.append(range->name);			continue; }
		if (*iter == continentColumn)	{ data.append(range->continent);	continue; }
		assert(false);
	}
	int newRangeIndex = NormalTable::addRow(parent, data);
	return newRangeIndex;
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
