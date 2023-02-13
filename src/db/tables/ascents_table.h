#ifndef ASCENTS_TABLE_H
#define ASCENTS_TABLE_H

#include "src/db/normal_table.h"
#include "src/data/ascent.h"

#include <QWidget>



class AscentsTable : public NormalTable {
	Q_OBJECT
	
public:
	Column* titleColumn;
	Column* peakIDColumn;
	Column* dateColumn;
	Column* peakOnDayColumn;
	Column* timeColumn;
	Column* hikeKindColumn;
	Column* traverseColumn;
	Column* difficultySystemColumn;
	Column* difficultyGradeColumn;
	Column* tripIDColumn;
	Column* descriptionColumn;
	
	AscentsTable(Column* foreignPeakIDColumn, Column* foreignTripIDColumn);
	
	int addRow(QWidget* parent, const Ascent* ascent);
};



#endif // ASCENTS_TABLE_H
