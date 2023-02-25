#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include "ui_about_window.h"

#include <QDialog>



class AboutWindow : public QDialog, public Ui_AboutWindow
{
	Q_OBJECT
	
public:
	AboutWindow(QWidget* parent);
};



#endif // ABOUT_WINDOW_H
