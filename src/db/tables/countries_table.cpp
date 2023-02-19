#include "countries_table.h"

#include <QString>
#include <QTranslator>



CountriesTable::CountriesTable() :
		NormalTable(QString("Countries"), tr("Countries"), "countryID"),
		//									name		uiName		type		nullable	primaryKey	foreignKey	inTable
		nameColumn	(new const Column(QString("name"),	tr("Name"),	varchar,	false,		false,		nullptr,	this))
{
	addColumn(nameColumn);
}



int CountriesTable::addRow(QWidget* parent, const Country* country)
{
	assert(country->countryID == -1);
	QList<QVariant> data = mapDataToQVariantList(country);
	
	int newCountryIndex = NormalTable::addRow(parent, data);
	return newCountryIndex;
}

void CountriesTable::updateRow(QWidget* parent, ValidItemID hikerID, const Country* country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(country);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> CountriesTable::mapDataToQVariantList(const Country* country) const
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = QList<QVariant>();
	for (const Column* column : columns) {
		if (column == nameColumn)	{ data.append(country->name);	continue; }
		assert(false);
	}
	return data;
}



QString CountriesTable::getNoneString() const
{
	return tr("None");
}

QString CountriesTable::getItemNameSingularLowercase() const
{
	return tr("country");
}

QString CountriesTable::getItemNamePluralLowercase() const
{
	return tr("countries");
}
