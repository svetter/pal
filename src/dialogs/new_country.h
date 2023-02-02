#ifndef NEW_COUNTRY_H
#define NEW_COUNTRY_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/country.h"
#include "ui_new_country.h"



class NewCountryDialog : public NewOrEditDialog, public Ui_NewCountryDialog
{
	Q_OBJECT
	
public:
	NewCountryDialog(QWidget* parent);
	
private:
	virtual bool changesMade();
	
	void handle_ok();
};



Country* openNewCountryDialogAndStore(QWidget* parent);
bool openEditCountryDialog(QWidget* parent, Country* country);



#endif // NEW_COUNTRY_H
