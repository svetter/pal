#include "participated_table.h"

#include <QTranslator>



ParticipatedTable::ParticipatedTable(Column* foreignAscentIDColumn, Column* foreignHikerIDColumn) :
		AssociativeTable(QString("Participated"), QString(), foreignAscentIDColumn, foreignHikerIDColumn)
{}



void ParticipatedTable::addRows(Ascent* ascent)
{
	// TODO
}
	