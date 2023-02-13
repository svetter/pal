#ifndef HIKERS_TABLE_H
#define HIKERS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/hiker.h"

#include <QWidget>



class HikersTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* nameColumn;
	
	HikersTable();
	
	int addRow(QWidget* parent, const Hiker* hiker);
};



#endif // HIKERS_TABLE_H
