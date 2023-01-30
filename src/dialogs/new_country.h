#ifndef NEW_COUNTRY_H
#define NEW_COUNTRY_H

#include "ui_new_country.h"



class NewCountryDialog : public QDialog, public Ui_NewCountryDialog
{
	Q_OBJECT
	
public:
	NewCountryDialog(QWidget *parent = 0);
	~NewCountryDialog();
	
private:
	Ui::NewCountryDialog *ui;
	void reject();
	
	bool anyChanges();
	
	void handle_close();
};



#endif // NEW_COUNTRY_H
