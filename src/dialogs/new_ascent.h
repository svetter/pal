#ifndef NEW_ASCENT_H
#define NEW_ASCENT_H

#include "ui_new_ascent.h"



class NewAscentDialog : public QDialog, public Ui_NewAscentDialog
{
	Q_OBJECT
	
public:
	NewAscentDialog(QWidget *parent = nullptr);
	~NewAscentDialog();
	
private:
	Ui::NewAscentDialog *ui;
	void reject();
	
	bool anyChanges();
	
	void handle_newPeak();
	void handle_dateSpecifiedChanged();
	void handle_timeSpecifiedChanged();
	void handle_difficultySystemChanged();
	void handle_newTrip();
	void handle_photosPathBrowse();
	void handle_close();
};



public Ascent* openNewAscentDialog();
public bool openEditAscentDialog(Ascent* ascent);



#endif // NEW_ASCENT_H
