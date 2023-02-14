#ifndef ASCENT_DIALOG_H
#define ASCENT_DIALOG_H

#include "src/dialogs/new_or_edit_dialog.h"
#include "src/data/ascent.h"
#include "ui_ascent_dialog.h"

#include <QAbstractTableModel>
#include <QList>
#include <QString>
#include <QStringList>
#include <QPair>



class HikersOnAscent;
class PhotosOfAscent;



class AscentDialog : public NewOrEditDialog, public Ui_AscentDialog
{
	Q_OBJECT
	
	const Ascent* init;
	
	HikersOnAscent hikersModel;
	PhotosOfAscent photosModel;
	
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
	void handle_removeHikers();
	void handle_addPhotos();
	void handle_removePhotos();
	
	void handle_ok();
};



int openNewAscentDialogAndStore(QWidget* parent, Database* db);
void openEditAscentDialogAndStore(QWidget* parent, Database* db, Ascent* originalAscent);



class HikersOnAscent : public QAbstractTableModel {
	QList<QPair<int, QString>> data;
	
public:
	HikersOnAscent();
	
	void addHiker(Hiker* hiker);
	void removeHiker(const QModelIndex& index);
	
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

class PhotosOfAscent : public QAbstractTableModel {
	QStringList data;
	
public:
	PhotosOfAscent();
	
	void addPhotos(const QStringList& photo);
	void removePhoto(const QModelIndex& index);
	
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};



#endif // ASCENT_DIALOG_H
