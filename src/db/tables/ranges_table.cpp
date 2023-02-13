#include "ranges_table.h"

#include <QString>
#include <QTranslator>



RangesTable::RangesTable() :
		NormalTable(QString("Ranges"), QString("range"), tr("Mountain ranges"), tr("None")),
		//							name					uiName				type		nullable	primaryKey	foreignKey	inTable
		nameColumn		(new Column(QString("name"),		tr("Name"),			varchar,	false,		false,		nullptr,	this)),
		continentColumn	(new Column(QString("continent"),	tr("Continent"),	integer,	true,		false,		nullptr,	this))
{
	addColumn(nameColumn);
	addColumn(continentColumn);
}



int RangesTable::addRow(QWidget* parent, const Range* range)
{
	QList<Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == nameColumn)		{ data.append(range->name);			continue; }
		if (*iter == continentColumn)	{ data.append(range->continent);	continue; }
		assert(false);
	}
	int newRangeIndex = NormalTable::addRow(parent, data);
	return newRangeIndex;
}
