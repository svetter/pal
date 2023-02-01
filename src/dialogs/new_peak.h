#ifndef NEW_PEAK_H
#define NEW_PEAK_H

#include "src/data/peak.h"
#include "ui_new_peak.h"



class NewPeakDialog : public QDialog, public Ui_NewPeakDialog
{
	Q_OBJECT
	
public:
	NewPeakDialog(QWidget *parent = nullptr);
	
private:
	void reject();
	
	bool anyChanges();
	
	void handle_heightSpecifiedChanged();
	void handle_newRegion();
	void handle_close();
};



Peak* openNewPeakDialogAndStore(QWidget *parent);
bool openEditPeakDialog(QWidget *parent, Peak* peak);



#endif // NEW_PEAK_H
