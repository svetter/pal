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
 * @file peak_dialog.h
 * 
 * This file declares the PeakDialog class.
 */

#ifndef PEAK_DIALOG_H
#define PEAK_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/peak.h"
#include "ui_peak_dialog.h"

#include <QNetworkReply>

using std::unique_ptr;



/**
 * Control class for the peak dialog.
 */
class PeakDialog : public ItemDialog, public Ui_PeakDialog
{
	Q_OBJECT
	
	/** The peak data before user interaction starts. */
	unique_ptr<const Peak> init;
	
	/** The list of IDs corresponding to the regions selectable in the region combo box. */
	QList<ValidItemID> selectableRegionIDs;
	
public:
	PeakDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Peak> init, int numItemsToEdit = -1);
	~PeakDialog();
	
	unique_ptr<Peak> extractData();
	
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_nameChanged();
	void handle_heightSpecifiedChanged();
	void handle_newRegion();
	void handle_mapsLinkChanged();
	void handle_earthLinkChanged();
	void handle_wikiLinkChanged();
	void handle_openMapsLink();
	void handle_openEarthLink();
	void handle_openWikiLink();
	void handle_findMapsLink();
	void handle_findEarthLink();
	void handle_findWikiLink();
	void handle_wikiLinkSearchResponse(QNetworkReply* reply);
	
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
	
	QString getUrlEscapedPeakName(QString spaceReplacement = "+");
};



void openNewPeakDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db, std::function<void (BufferRowIndex)> callWhenDone);
void openDuplicatePeakDialogAndStore	(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (BufferRowIndex)> callWhenDone);
void openEditPeakDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (bool)> callWhenDone);
void openMultiEditPeaksDialogAndStore	(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndex, BufferRowIndex initBufferRowIndex, std::function<void (bool)> callWhenDone);
bool openDeletePeaksDialogAndExecute	(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices);



#endif // PEAK_DIALOG_H
