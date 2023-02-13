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
	CountryDialog(QWidget* parent, Database* db, Country* init = nullptr);
	~CountryDialog();
	
	Country* extractData();
	virtual bool changesMade();
	
private:
	void insertInitData();
	
	void handle_ok();
};



int openNewCountryDialogAndStore(QWidget* parent, Database* db);
void openEditCountryDialogAndStore(QWidget* parent, Database* db, Country* originalCountry);



#endif // COUNTRY_DIALOG_H
