#ifndef ASCENTS_TABLE_H
#define ASCENTS_TABLE_H

#include "src/db/db_model.h"
#include "src/data/ascent.h"



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
	Column* notesColumn;
	
	AscentsTable(Column* foreignPeakIDColumn, Column* foreignTripIDColumn);
	
	int addRow(Ascent* ascent);
};



#endif // ASCENTS_TABLE_H
