#ifndef COUNTRY_DIALOG_H
#define COUNTRY_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/country.h"
#include "ui_country_dialog.h"



class CountryDialog : public NewOrEditDialog, public Ui_CountryDialog
{
	Q_OBJECT
	
	Country* init;
	
public:
	CountryDialog(QWidget* parent, Database* db, Country* init = nullptr);
	~CountryDialog();
	
	Country* extractData();
	
private:
	void insertInitData();
	virtual bool changesMade();
	
	void handle_ok();
};



Country* openNewCountryDialogAndStore(QWidget* parent, Database* db);
bool openEditCountryDialog(QWidget* parent, Database* db, Country* country);



#endif // COUNTRY_DIALOG_H
