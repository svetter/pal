#ifndef NEW_PEAK_H
#define NEW_PEAK_H

#include "ui_new_peak.h"



class NewPeakDialog : public QDialog, public Ui_NewPeakDialog
{
	Q_OBJECT
public:
	NewPeakDialog(QWidget *parent = 0);
private slots:
	void on_cancelButton_clicked();
	void on_newRangeButton_clicked();
};



#endif // NEW_PEAK_H
