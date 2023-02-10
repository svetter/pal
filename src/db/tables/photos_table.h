#ifndef PHOTOS_TABLE_H
#define PHOTOS_TABLE_H

#include "src/db/db_model.h"
#include "src/data/ascent.h"

#include <QString>
#include <QTranslator>



class PhotosTable : public NormalTable {
public:
	Column* ascentIDColumn;
	Column* indexColumn;
	Column* filepathColumn;
	
	PhotosTable(Column* foreignAscentIDColumn);
	
	int addRow();
};



#endif // PHOTOS_TABLE_H
