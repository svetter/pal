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
 * @file country_dialog.h
 * 
 * This file declares the CountryDialog class.
 */

#ifndef COUNTRY_DIALOG_H
#define COUNTRY_DIALOG_H

#include "src/dialogs/item_dialog.h"
#include "src/data/country.h"
#include "ui_country_dialog.h"

using std::unique_ptr;



/**
 * Control class for the country dialog.
 */
class CountryDialog : public ItemDialog, public Ui_CountryDialog
{
	Q_OBJECT
	
	/** The country data before user interaction starts. */
	unique_ptr<const Country> init;
	
public:
	CountryDialog(QWidget& parent, QMainWindow& mainWindow, Database& db, DialogPurpose purpose, unique_ptr<const Country> init, int numItemsToEdit = -1);
	~CountryDialog();
	
	unique_ptr<Country> extractData();
	
	virtual bool changesMade() override;
	
private:
	void insertInitData();
	
private slots:
	void handle_ok() override;
	
private:
	virtual void aboutToClose() override;
};



void openNewCountryDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db, std::function<void (BufferRowIndex)> callWhenDone);
void openEditCountryDialogAndStore			(QWidget& parent, QMainWindow& mainWindow, Database& db, BufferRowIndex bufferRowIndex, std::function<void (bool)> callWhenDone);
void openMultiEditCountriesDialogAndStore	(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndex, BufferRowIndex initBufferRowIndex, std::function<void (bool)> callWhenDone);
bool openDeleteCountriesDialogAndExecute	(QWidget& parent, QMainWindow& mainWindow, Database& db, const QSet<BufferRowIndex>& bufferRowIndices);



#endif // COUNTRY_DIALOG_H
