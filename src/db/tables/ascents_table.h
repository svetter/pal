#ifndef ASCENTS_TABLE_H
#define ASCENTS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QWidget>



class AscentsTable : public NormalTable {
	Q_OBJECT
	
public:
	const Column* titleColumn;
	const Column* peakIDColumn;
	const Column* dateColumn;
	const Column* peakOnDayColumn;
	const Column* timeColumn;
	const Column* elevationGainColumn;
	const Column* hikeKindColumn;
	const Column* traverseColumn;
	const Column* difficultySystemColumn;
	const Column* difficultyGradeColumn;
	const Column* tripIDColumn;
	const Column* descriptionColumn;
	
	AscentsTable(const Column* foreignPeakIDColumn, const Column* foreignTripIDColumn);
	
	int addRow(QWidget* parent, Ascent* ascent);
	void updateRow(QWidget* parent, ValidItemID ascentID, const Ascent* ascent);
	
	QList<QVariant> mapDataToQVariantList(const Ascent* ascent) const;
	
	virtual QString getNoneString() const;
	virtual QString getItemNameSingularLowercase() const;
	virtual QString getItemNamePluralLowercase() const;
};



#endif // ASCENTS_TABLE_H
