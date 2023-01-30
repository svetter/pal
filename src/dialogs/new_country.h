#ifndef NEW_COUNTRY_H
#define NEW_COUNTRY_H

#include "ui_new_country.h"



class NewCountryDialog : public QDialog, public Ui_NewCountryDialog
{
	Q_OBJECT
public:
	NewCountryDialog(QWidget *parent = 0);
private slots:
	void on_cancelButton_clicked();
};



#endif // NEW_COUNTRY_H
