#ifndef HIKER_LIST_H
#define HIKER_LIST_H

#include "src/data/hiker.h"

#include <QAbstractTableModel>
#include <QList>
#include <QPair>
#include <QString>



class HikersOnAscent : public QAbstractTableModel {
	QList<QPair<int, QString>> list;
	
public:
	HikersOnAscent();
	
	void addHiker(Hiker* hiker);
	void removeHikerAt(int rowIndex);
	
	QList<int> getHikerIDList() const;
	
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};



#endif // HIKER_LIST_H