#ifndef PHOTOS_TABLE_H
#define PHOTOS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>
#include <QTranslator>



class PhotosTable : public NormalTable {
public:
	Column* ascentIDColumn;
	Column* sortIndexColumn;
	Column* filepathColumn;
	
	PhotosTable(Column* foreignAscentIDColumn);
	
	QStringList getPhotosForAscent(int ascentID) const;
	
	void addRows(QWidget* parent, const Ascent* ascent);
	
private:
	void addRow(QWidget* parent, int ascentID, int sortIndex, const QString& filepath);
};



#endif // PHOTOS_TABLE_H
