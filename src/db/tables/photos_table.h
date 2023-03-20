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
	Column* const ascentIDColumn;
	Column* const sortIndexColumn;
	Column* const filepathColumn;
	Column* const descriptionColumn;
	
	PhotosTable(Column* foreignAscentIDColumn);
	
	QList<Photo> getPhotosForAscent(ValidItemID ascentID) const;
	
	void addRows(QWidget* parent, ValidItemID ascentID, const QList<Photo>& photos);
	void addRows(QWidget* parent, const Ascent* ascent);
	void updateRows(QWidget* parent, ValidItemID ascentID, const QList<Photo>& photos);
	void updateRows(QWidget* parent, const Ascent* ascent);
	void updateFilepathAt(QWidget* parent, int bufferRowIndex, QString newFilepath);
	void removeRowsForAscent(QWidget* parent, ValidItemID ascentID);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, ValidItemID ascentID, int sortIndex, const QString& filepath, const QString& description) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // PHOTOS_TABLE_H
