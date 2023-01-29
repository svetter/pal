#ifndef ADDASCENT_H
#define ADDASCENT_H

#include "ui_addascent.h"



class AddAscentDialog : public QDialog, public Ui_AddAscentDialog
{
	Q_OBJECT
public:
	AddAscentDialog(QWidget *parent = 0);
};



#endif // ADDASCENT_H
