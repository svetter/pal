/*
 * Copyright 2023-2024 Simon Vetter
 * 
 * This file is part of PeakAscentLogger.
 * 
 * PeakAscentLogger is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * PeakAscentLogger is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with PeakAscentLogger.
 * If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file photos_table.h
 * 
 * This file declares the PhotosTable class.
 */

#ifndef PHOTOS_TABLE_H
#define PHOTOS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QString>
#include <QWidget>
#include <QList>
#include <QVariant>



/**
 * A class for accessing and manipulating the photos table in the database.
 */
class PhotosTable : public NormalTable {
	Q_OBJECT
	
public:
	/** The ascent ID column. */
	ForeignKeyColumn ascentIDColumn;
	/** The sort index column. */
	ValueColumn sortIndexColumn;
	/** The file path column. */
	ValueColumn filepathColumn;
	/** The description column. */
	ValueColumn descriptionColumn;
	
	PhotosTable(Database& db, PrimaryKeyColumn& foreignAscentIDColumn);
	
	QList<Photo> getPhotosForAscent(ValidItemID ascentID) const;
	
	void addRows(QWidget& parent, ValidItemID ascentID, const QList<Photo>& photos);
	void addRows(QWidget& parent, const Ascent& ascent);
	void updateRows(QWidget& parent, ValidItemID ascentID, const QList<Photo>& photos);
	void updateRows(QWidget& parent, const Ascent& ascent);
	void updateRows(QWidget& parent, const QSet<ValidItemID>& ascentIDs, const QList<Photo>& photos);
	void updateFilepathAt(QWidget& parent, BufferRowIndex bufferRowIndex, QString newFilepath);
private:
	const QList<ColumnDataPair> mapDataToColumnDataPairs(const QList<const Column*>& columns, ValidItemID ascentID, int sortIndex, const QString& filepath, const QString& description) const;
	
public:
	virtual QString getIdentityRepresentationAt(const BufferRowIndex& bufferRow) const override;
	virtual QList<const Column*> getIdentityRepresentationColumns() const override;
	
	virtual QString getItemNameSingular() const override;
	virtual QString getNoneString() const override;
	virtual QString getCreationConfirmMessage() const override;
	virtual QString getEditConfirmMessage(int numEdited) const override;
	virtual QString getDeleteConfirmMessage(int numDeleted) const override;
	virtual QString getNewCustomColumnString() const override;
	virtual QString getNewFilterString() const override;
	virtual QString getItemCountString(int numItems) const override;
};



#endif // PHOTOS_TABLE_H
