#include "participated_table.h"

#include <QTranslator>



ParticipatedTable::ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn) :
		AssociativeTable(QString("Participated"), foreignAscentIDColumn, foreignHikerIDColumn),
		ascentIDColumn(getColumn1()),
		hikerIDColumn(getColumn2())
{}



void ParticipatedTable::addRows(Ascent* ascent)
{
	// TODO #97
}
	