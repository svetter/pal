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
