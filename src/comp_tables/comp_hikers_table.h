#ifndef COMP_HIKERS_TABLE_H
#define COMP_HIKERS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeHikersTable : public CompositeTable {
public:
	const DirectCompositeColumn*	nameColumn;
	const FoldCompositeColumn*		numAscentsColumn;
	const FoldCompositeColumn*		numTripsColumn;
	const FoldCompositeColumn*		avgElevationGainColumn;
	const FoldCompositeColumn*		maxElevationGainColumn;
	const FoldCompositeColumn*		sumElevationGainColumn;
	
public:
	inline CompositeHikersTable(Database* db) :
			CompositeTable(db, db->hikersTable),
			//																uiName				align/fold op	breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn	(tr("Hiker name"),		Qt::AlignLeft,	db->hikersTable->nameColumn)),
			numAscentsColumn		(new const FoldCompositeColumn		(tr("Num. ascents"),	Count,			{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn} })),
			numTripsColumn			(new const FoldCompositeColumn		(tr("Num. trips"),		Count,			{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn},	{db->ascentsTable->tripIDColumn,		db->tripsTable->primaryKeyColumn} })),
			avgElevationGainColumn	(new const FoldCompositeColumn		(tr("Avg. elev. gain"),	Average,		{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn)),
			maxElevationGainColumn	(new const FoldCompositeColumn		(tr("Max. elev. gain"),	Max,			{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn)),
			sumElevationGainColumn	(new const FoldCompositeColumn		(tr("Sum elev. gain"),	Sum,			{ {db->hikersTable->primaryKeyColumn,	db->participatedTable->hikerIDColumn},		{db->participatedTable->ascentIDColumn,	db->ascentsTable->primaryKeyColumn} },	db->ascentsTable->elevationGainColumn))
	{
		addColumn(nameColumn);
		addColumn(numAscentsColumn);
		addColumn(numTripsColumn);
		addColumn(avgElevationGainColumn);
		addColumn(maxElevationGainColumn);
		addColumn(sumElevationGainColumn);
	}
};



#endif // COMP_HIKERS_TABLE_H
