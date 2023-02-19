#ifndef PHOTOS_TABLE_H
#define PHOTOS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>
#include <QTranslator>



class PhotosTable : public NormalTable {
public:
	const Column* ascentIDColumn;
	const Column* sortIndexColumn;
	const Column* filepathColumn;
	
	PhotosTable(const Column* foreignAscentIDColumn);
	
	QStringList getPhotosForAscent(ValidItemID ascentID) const;
	
	void addRows(QWidget* parent, const Ascent* ascent);
	
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
	
private:
	void addRow(QWidget* parent, ValidItemID ascentID, int sortIndex, const QString& filepath);
};



#endif // PHOTOS_TABLE_H
