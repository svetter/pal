#ifndef COUNTRY_DIALOG_H
#define COUNTRY_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/country.h"
#include "ui_country_dialog.h"



class CountryDialog : public NewOrEditDialog, public Ui_CountryDialog
{
	Q_OBJECT
	
public:
	CountryDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_ok();
};



Country* openNewCountryDialogAndStore(QWidget* parent);
bool openEditCountryDialog(QWidget* parent, Country* country);



#endif // COUNTRY_DIALOG_H
