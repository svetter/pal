#ifndef PHOTOS_TABLE_H
#define PHOTOS_TABLE_H

#include "src/db/db_model.h"

#include <QString>
#include <QTranslator>



class PhotosTable : public NormalTable {
public:
	Column* ascentIDColumn;
	Column* indexColumn;
	Column* filepathColumn;
	
	PhotosTable(Column* foreignAscentIDColumn);
	
	void addRows(Ascent* ascent);
};



#endif // PHOTOS_TABLE_H
