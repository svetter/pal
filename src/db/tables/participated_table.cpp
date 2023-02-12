#include "participated_table.h"

#include <QTranslator>



ParticipatedTable::ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn) :
		AssociativeTable(QString("Participated"), foreignAscentIDColumn, foreignHikerIDColumn)
{}



void ParticipatedTable::addRows(Ascent* ascent)
{
	// TODO #97
}
	