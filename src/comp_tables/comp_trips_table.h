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
	
public:
	inline CompositeTripsTable(Database* db) :
			CompositeTable(db, db->tripsTable),
			//																	uiName				align/fold op	breadcrumbs (column reference chain) + content column [+ enum names]
			nameColumn				(new const DirectCompositeColumn		(tr("Trip name"),		Qt::AlignLeft,	db->tripsTable->nameColumn)),
			startDateColumn			(new const DirectCompositeColumn		(tr("Start date"),		Qt::AlignLeft,	db->tripsTable->startDateColumn)),
			endDateColumn			(new const DirectCompositeColumn		(tr("End date"),		Qt::AlignLeft,	db->tripsTable->endDateColumn)),
			lengthColumn			(new const DifferenceCompositeColumn	(tr("Length"),							db->tripsTable->endDateColumn,			db->tripsTable->startDateColumn,		tr(" days"))),
			numAscentsColumn		(new const FoldCompositeColumn			(tr("Num. ascents"),	Count,			{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} })),
			avgElevationGainColumn	(new const FoldCompositeColumn			(tr("Avg. elev. gain"),	Average,		{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			maxElevationGainColumn	(new const FoldCompositeColumn			(tr("Max. elev. gain"),	Max,			{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn)),
			sumElevationGainColumn	(new const FoldCompositeColumn			(tr("Sum elev. gain"),	Sum,			{ {db->tripsTable->primaryKeyColumn,	db->ascentsTable->tripIDColumn} },		db->ascentsTable->elevationGainColumn))
	{
		addColumn(nameColumn);
		addColumn(startDateColumn);
		addColumn(endDateColumn);
		addColumn(lengthColumn);
		addColumn(numAscentsColumn);
		addColumn(avgElevationGainColumn);
		addColumn(maxElevationGainColumn);
		addColumn(sumElevationGainColumn);
	}
};



#endif // COMP_TRIPS_TABLE_H
