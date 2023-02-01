#include "db_model.h"

#include <QString>



class AscentTable : NormalTable {
public:
	Column<QString>				titleColumn;
	Column<Peak*>				peakColumn;
	Column<Date>				dateColumn;
	Column<int>					peakOnDayColumn;
	Column<Time>				timeColumn;
	Column<HikeKind>			kindColumn;
	Column<bool>				traverseColumn;
	Column<DifficultySystem>	difficultySystemColumn;
	Column<DifficultyGrade>		difficultyGradeColumn;
	Column<Trip*>				tripColumn;
	Column<QString>				notesColumn;
	
	AscentTable();
	
	void addRow(Ascent* ascent);
	WhatIfResult whatIf_removeRow(int ascentID);
	void removeRow(int ascentID);
	WhatIfResult whatIf_changeCell(int ascentID, Column* column);
	template<class T> void changeCell(int ascentID, Column<T>* column, T newValue);
}


class PeakTable : NormalTable {
public:
	Column<QString>	nameColumn;
	Column<int>		heightColumn;
	Column<bool>	volcanoColumn;
	Column<Region*>	regionColumn;
	Column<QString>	mapsLinkColumn;
	Column<QString>	earthLinkColumn;
	Column<QString>	wikiLinkColumn;
	
	PeakTable();
	
	void addRow(Peak* peak);
	WhatIfResult whatIf_removeRow(int ascentID);
	void removeRow(int ascentID);
	WhatIfResult whatIf_changeCell(int ascentID, Column* column);
	template<class T> void changeCell(int ascentID, Column<T>* column, T newValue);
}
