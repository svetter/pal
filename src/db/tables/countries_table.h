#ifndef COUNTRIES_TABLE_H
#define COUNTRIES_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/country.h"

#include <QWidget>



class CountriesTable : public NormalTable {
	Q_OBJECT
	
public:
	const Column* nameColumn;
	
	CountriesTable();
	
	int addRow(QWidget* parent, Country* country);
	void updateRow(QWidget* parent, ValidItemID countryID, const Country* country);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Country* country) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // COUNTRIES_TABLE_H
