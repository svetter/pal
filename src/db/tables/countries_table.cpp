#include "countries_table.h"

#include <QString>
#include <QTranslator>



CountriesTable::CountriesTable() :
		NormalTable(QString("Countries"), tr("Countries"), "countryID"),
		//									name		uiName		type		nullable	primaryKey	foreignKey	inTable
		nameColumn	(new const Column(QString("name"),	tr("Name"),	varchar,	false,		false,		nullptr,	this))
{
	addColumn(primaryKeyColumn);
	addColumn(nameColumn);
}



int CountriesTable::addRow(QWidget* parent, Country* country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, country);
	
	int newCountryIndex = NormalTable::addRow(parent, columns, data);
	country->countryID = getPrimaryKeyAt(newCountryIndex);
	return newCountryIndex;
}

void CountriesTable::updateRow(QWidget* parent, ValidItemID hikerID, const Country* country)
{
	QList<const Column*> columns = getNonPrimaryKeyColumnList();
	QList<QVariant> data = mapDataToQVariantList(columns, country);
	
	NormalTable::updateRow(parent, hikerID, columns, data);
}


QList<QVariant> CountriesTable::mapDataToQVariantList(QList<const Column*>& columns, const Country* country) const
{
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
