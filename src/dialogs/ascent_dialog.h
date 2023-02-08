#ifndef ASCENT_DIALOG_H
#define ASCENT_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/ascent.h"
#include "ui_ascent_dialog.h"



class AscentDialog : public NewOrEditDialog, public Ui_AscentDialog
{
	Q_OBJECT
	
	Ascent* init;
	
public:
	AscentDialog(QWidget* parent, Database* db, Ascent* init = nullptr);
	~AscentDialog();
	
	Ascent* extractData();
	virtual bool changesMade();
	
private:
	void populateComboBoxes();
	void insertInitData();
	
	void handle_newPeak();
	void handle_dateSpecifiedChanged();
	void handle_timeSpecifiedChanged();
	void handle_difficultySystemChanged();
	void handle_newTrip();
	void handle_addHiker();
	void handle_photosPathBrowse();
	
	void handle_ok();
};



Ascent* openNewAscentDialogAndStore(QWidget* parent, Database* db);
bool openEditAscentDialog(QWidget* parent, Database* db, Ascent* originalAscent);



#endif // ASCENT_DIALOG_H
