#include "hikers_table.h"

#include <QString>
#include <QTranslator>



HikersTable::HikersTable() :
		NormalTable(QString("Hikers"), tr("Hikers"), "hikerID"),
		//									name		uiName		type		nullable	primaryKey	foreignKey	inTable
		nameColumn	(new const Column(QString("name"),	tr("Name"),	varchar,	false,		false,		nullptr,	this))
{
	addColumn(nameColumn);
}



int HikersTable::addRow(QWidget* parent, const Hiker* hiker)
{
	assert(hiker->hikerID == -1);
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == nameColumn)	{ data.append(hiker->name);	continue; }
		assert(false);
	}
	int newHikerIndex = NormalTable::addRow(parent, data);
	return newHikerIndex;
}



QString HikersTable::getNoneString() const
{
	return tr("None");
}

QString HikersTable::getItemNameSingularLowercase() const
{
	return tr("hiker");
}

QString HikersTable::getItemNamePluralLowercase() const
{
	return tr("hikers");
}
