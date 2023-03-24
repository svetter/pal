#ifndef ASCENT_DIALOG_H
#define ASCENT_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/ascent.h"
#include "ui_ascent_dialog.h"

#include "list/hiker_list.h"
#include "list/photo_list.h"

#include <QWidget>



class AscentDialog : public NewOrEditDialog, public Ui_AscentDialog
{
	Q_OBJECT
	
	const Ascent* init;
	
	QList<ValidItemID> selectablePeakIDs;
	QList<ValidItemID> selectableTripIDs;
	
	HikersOnAscent hikersModel;
	PhotosOfAscent photosModel;
	
public:
	AscentDialog(QWidget* parent, Database* db, DialogPurpose purpose, Ascent* init);
	~AscentDialog();
	
	virtual QString getEditWindowTitle() override;
	
	Ascent* extractData();
	virtual bool changesMade() override;
	
private:
	void populateComboBoxes();
	void insertInitData();
	
private slots:
	void handle_newPeak();
	void handle_dateSpecifiedChanged();
	void handle_timeSpecifiedChanged();
	void handle_elevationGainSpecifiedChanged();
	void handle_difficultySystemChanged();
	void handle_newTrip();
	void handle_addHiker();
	void handle_removeHikers();
	void handle_addPhotos();
	void handle_removePhotos();
	void handle_photoSelectionChanged(const QItemSelection& selected = QItemSelection(), const QItemSelection& deselected = QItemSelection());
	
	void handle_ok();
	
private:
	virtual void aboutToClose() override;
	
	QSet<int> getPreviouslySelectedRows(const QItemSelection& selected, const QItemSelection& deselected) const;
};



int openNewAscentDialogAndStore			(QWidget* parent, Database* db);
int openDuplicateAscentDialogAndStore	(QWidget* parent, Database* db, int bufferRowIndex);
void openEditAscentDialogAndStore		(QWidget* parent, Database* db, int bufferRowIndex);
void openDeleteAscentDialogAndExecute	(QWidget* parent, Database* db, int bufferRowIndex);

QStringList openFileDialogForPhotosSelection(QWidget* parent);



#endif // ASCENT_DIALOG_H
