#include "countries_table.h"

#include <QString>
#include <QTranslator>



CountriesTable::CountriesTable() :
		NormalTable(QString("Countries"), QString("country"), tr("Countries"), tr("None")),
		//						name				uiName		type		nullable	primaryKey	foreignKey	inTable
		nameColumn	(new Column(QString("name"),	tr("Name"),	varchar,	false,		false,		nullptr,	this))
{
	addColumn(nameColumn);
}



int CountriesTable::addRow(QWidget* parent, const Country* country)
{
	assert(country->countryID == -1);
	QList<Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (auto iter = columns.constBegin(); iter != columns.constEnd(); iter++) {
		if (*iter == nameColumn)	{ data.append(country->name);	continue; }
		assert(false);
	}
	int newCountryIndex = NormalTable::addRow(parent, data);
	return newCountryIndex;
}