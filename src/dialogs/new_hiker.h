#ifndef NEW_HIKER_H
#define NEW_HIKER_H

#include "ui_new_hiker.h"



class NewHikerDialog : public QDialog, public Ui_NewHikerDialog
{
	Q_OBJECT
public:
	NewHikerDialog(QWidget *parent = 0);
private slots:
	void on_cancelButton_clicked();
};



#endif // NEW_HIKER_H
