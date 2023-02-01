#ifndef NEW_COUNTRY_H
#define NEW_COUNTRY_H

#include "src/data/country.h"
#include "ui_new_country.h"



class NewCountryDialog : public QDialog, public Ui_NewCountryDialog
{
	Q_OBJECT
	
public:
	NewCountryDialog(QWidget *parent = 0);
	
private:
	void reject();
	
	bool anyChanges();
	
	void handle_ok();
	void handle_cancel();
};



Country* openNewCountryDialogAndStore(QWidget *parent);
bool openEditCountryDialog(QWidget *parent, Country* country);



#endif // NEW_COUNTRY_H
