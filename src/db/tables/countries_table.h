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
	
	int addRow(QWidget* parent, const Country* country);
};



#endif // COUNTRIES_TABLE_H
