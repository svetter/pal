#ifndef DB_TABLES_H
#define DB_TABLES_H

#include "src/db/db_model.h"
#include "src/data/ascent.h"
#include "src/data/peak.h"
#include "src/data/trip.h"
#include "src/data/hiker.h"
#include "src/data/region.h"
#include "src/data/range.h"
#include "src/data/country.h"
#include "src/db/db_model.h"

#include <QString>



class AscentTable : NormalTable {
public:
	Column titleColumn;
	Column peakColumn;
	Column dateColumn;
	Column peakOnDayColumn;
	Column timeColumn;
	Column kindColumn;
	Column traverseColumn;
	Column difficultySystemColumn;
	Column difficultyGradeColumn;
	Column tripColumn;
	Column notesColumn;
	
	AscentTable();
	
	void addRow(Ascent* ascent);
};


class PeakTable : NormalTable {
public:
	Column nameColumn;
	Column heightColumn;
	Column volcanoColumn;
	Column regionColumn;
	Column mapsLinkColumn;
	Column earthLinkColumn;
	Column wikiLinkColumn;
	
	PeakTable();
	
	void addRow(Peak* peak);
};



#endif // DB_TABLES_H
