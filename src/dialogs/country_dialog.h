/*
 * Copyright 2023 Simon Vetter
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

#ifndef COUNTRY_DIALOG_H
#define COUNTRY_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/country.h"
#include "ui_country_dialog.h"



class CountryDialog : public NewOrEditDialog, public Ui_CountryDialog
{
	Q_OBJECT
	
	const Country* init;
	
public:
	CountryDialog(QWidget* parent, Database* db, DialogPurpose purpose, Country* init);
	~CountryDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Country* extractData();
	virtual bool changesMade() override;
	
private:
	void insertInitData();
	
private slots:
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
};



int openNewCountryDialogAndStore		(QWidget* parent, Database* db);
void openEditCountryDialogAndStore		(QWidget* parent, Database* db, int bufferRowIndex);
void openDeleteCountryDialogAndExecute	(QWidget* parent, Database* db, int bufferRowIndex);



#endif // COUNTRY_DIALOG_H
