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
	QList<QVariant> data = mapDataToQVariantList(hiker);
	
	int newHikerIndex = NormalTable::addRow(parent, data);
	return newHikerIndex;
}

void HikersTable::updateRow(QWidget* parent, ValidItemID hikerID, const Hiker* hiker)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(hiker);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> HikersTable::mapDataToQVariantList(const Hiker* hiker) const
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)	{ data.append(hiker->name);	continue; }
		assert(false);
	}
	return data;
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
