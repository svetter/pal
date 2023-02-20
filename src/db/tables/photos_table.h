#ifndef PHOTOS_TABLE_H
#define PHOTOS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>
#include <QList>
#include <QVariant>



class PhotosTable : public NormalTable {
public:
	const Column* ascentIDColumn;
	const Column* sortIndexColumn;
	const Column* filepathColumn;
	
	PhotosTable(const Column* foreignAscentIDColumn);
	
	QStringList getPhotosForAscent(ValidItemID ascentID) const;
	
	void addRows(QWidget* parent, const Ascent* ascent);
	void updateRows(QWidget* parent, const Ascent* ascent);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, ValidItemID ascentID, int sortIndex, const QString& filepath) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // PHOTOS_TABLE_H
