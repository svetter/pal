#ifndef COUNTRIES_TABLE_H
#define COUNTRIES_TABLE_H

#include "src/db/db_model.h"
#include "src/data/country.h"



class CountriesTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* nameColumn;
	
	CountriesTable();
	
	int addRow(Country* country);
};



#endif // COUNTRIES_TABLE_H
