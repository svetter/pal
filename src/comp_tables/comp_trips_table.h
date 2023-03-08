#ifndef COMP_TRIPS_TABLE_H
#define COMP_TRIPS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeTripsTable : public CompositeTable {
public:
	const DirectCompositeColumn*		nameColumn;
	const DirectCompositeColumn*		startDateColumn;
	const DirectCompositeColumn*		endDateColumn;
	const DifferenceCompositeColumn*	lengthColumn;
	const FoldCompositeColumn*			numAscentsColumn;
	const FoldCompositeColumn*			avgElevationGainColumn;
	const FoldCompositeColumn*			maxElevationGainColumn;
	const FoldCompositeColumn*			sumElevationGainColumn;
	const FoldCompositeColumn*			listHikersColumn;
	
public:
	inline CompositeTripsTable(Database* db) :
			CompositeTable(db, db->tripsTable),
			//																			uiName				align/fold op	breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn		(this,	tr("Trip name"),		Qt::AlignLeft,	db->tripsTable->nameColumn)),
			startDateColumn			(new const DirectCompositeColumn		(this,	tr("Start date"),		Qt::AlignLeft,	db->tripsTable->startDateColumn)),
			endDateColumn			(new const DirectCompositeColumn		(this,	tr("End date"),			Qt::AlignLeft,	db->tripsTable->endDateColumn)),
			lengthColumn			(new const DifferenceCompositeColumn	(this,	tr("Length"),							db->tripsTable->endDateColumn,			db->tripsTable->startDateColumn,		tr(" days"))),
			numAscentsColumn		(new const FoldCompositeColumn			(this,	tr("Num. ascents"),		Count,			{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} })),
			avgElevationGainColumn	(new const FoldCompositeColumn			(this,	tr("Avg. elev. gain"),	Average,		{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			maxElevationGainColumn	(new const FoldCompositeColumn			(this,	tr("Max. elev. gain"),	Max,			{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			sumElevationGainColumn	(new const FoldCompositeColumn			(this,	tr("Sum elev. gain"),	Sum,			{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			listHikersColumn		(new const FoldCompositeColumn			(this,	tr("Participants"),		List,			{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn},		{db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn},	{db->participatedTable->hikerIDColumn,	db->hikersTable->primaryKeyColumn} },	db->hikersTable->nameColumn))
	{
		addColumn(nameColumn);
		addColumn(startDateColumn);
		addColumn(endDateColumn);
		addColumn(lengthColumn);
		addColumn(numAscentsColumn);
		addColumn(avgElevationGainColumn);
		addColumn(maxElevationGainColumn);
		addColumn(sumElevationGainColumn);
		addColumn(listHikersColumn);
		
		initBuffer();
	}
};



#endif // COMP_TRIPS_TABLE_H
