#ifndef HIKERS_TABLE_H
#define HIKERS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/hiker.h"

#include <QWidget>



class HikersTable : public NormalTable {
	Q_OBJECT
	
public:
	const Column* nameColumn;
	
	HikersTable();
	
	int addRow(QWidget* parent, const Hiker* hiker);
	void updateRow(QWidget* parent, ValidItemID hikerID, const Hiker* hiker);
	
	QList<QVariant> mapDataToQVariantList(const Hiker* hiker) const;
	
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // HIKERS_TABLE_H
