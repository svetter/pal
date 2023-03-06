#ifndef ASCENTS_TABLE_H
#define ASCENTS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QWidget>



class AscentsTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* const titleColumn;
	Column* const peakIDColumn;
	Column* const dateColumn;
	Column* const peakOnDayColumn;
	Column* const timeColumn;
	Column* const elevationGainColumn;
	Column* const hikeKindColumn;
	Column* const traverseColumn;
	Column* const difficultySystemColumn;
	Column* const difficultyGradeColumn;
	Column* const tripIDColumn;
	Column* const descriptionColumn;
	
	AscentsTable(Column* foreignPeakIDColumn, Column* foreignTripIDColumn);
	
	int addRow(QWidget* parent, Ascent* ascent);
	void updateRow(QWidget* parent, const Ascent* ascent);
private:
	QList<QVariant> mapDataToQVariantList(QList<const Column*>& columns, const Ascent* ascent) const;
	
public:
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // ASCENTS_TABLE_H
