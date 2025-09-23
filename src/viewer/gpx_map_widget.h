/*
 * Copyright 2023-2025 Simon Vetter
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


#ifndef GPX_MAP_WIDGET_H
#define GPX_MAP_WIDGET_H

#include "src/data/item_id.h"
#include "src/db/database.h"
#include "src/viewer/gpx_file_server.h"
#include "ui_gpx_map_widget.h"



class GpxMapWidget : public QWidget, public Ui_GpxMapWidget
{
	Q_OBJECT
	
	Database* db;
	
	GpxFileServer gpxFileServer;
	
	const ItemID* currentAscentID;
	
public:
	GpxMapWidget(QWidget* parent);
	virtual ~GpxMapWidget();
	
	void supplyPointers(Database* const db, const ItemID* const currentAscentID);
	
	void ascentAboutToChange();
	void ascentChanged();
	
	void aboutToExit();
	
private slots:
	void handle_filepathChanged();
	void handle_browseButtonClicked();
	void handle_filesDropped(QStringList filepaths);
	
private:
	void updateFileDropFrameProperties(bool validAscent, bool fileSet, bool fileExists);
	QString createGpxStudioEmbedUrl(const QString& serverFilename);
	void saveFilepath();
};



#endif // GPX_MAP_WIDGET_H
