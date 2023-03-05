#ifndef COMP_ASCENTS_TABLE_H
#define COMP_ASCENTS_TABLE_H

#include "src/comp_tables/composite_table.h"



class CompositeAscentsTable : public CompositeTable {
public:
	const DirectCompositeColumn*		dateColumn;
	const DirectCompositeColumn*		titleColumn;
	const ReferenceCompositeColumn*		peakColumn;
	const ReferenceCompositeColumn*		peakHeightColumn;
	const ReferenceCompositeColumn*		regionColumn;
	const ReferenceCompositeColumn*		rangeColumn;
	const ReferenceCompositeColumn*		countryColumn;
	const ReferenceCompositeColumn*		continentColumn;
	const ReferenceCompositeColumn*		volcanoColumn;
	const DirectCompositeColumn*		elevationGainColumn;
	const DirectCompositeColumn*		hikeKindColumn;
	const DirectCompositeColumn*		traverseColumn;
	const DependentEnumCompositeColumn*	difficultyGradeColumn;
	const FoldCompositeColumn*			hikersColumn;
	const ReferenceCompositeColumn*		tripColumn;
	
public:
	inline CompositeAscentsTable(Database* db) :
			CompositeTable(db, db->ascentsTable),
			//																	uiName				align/fold op		breadcrumbs (column reference chain) + content column [+ enum names]
			dateColumn				(new const DirectCompositeColumn		(tr("Date"),			Qt::AlignLeft,		db->ascentsTable->dateColumn)),
			titleColumn				(new const DirectCompositeColumn		(tr("Title"),			Qt::AlignLeft,		db->ascentsTable->titleColumn)),
			peakColumn				(new const ReferenceCompositeColumn		(tr("Peak"),			Qt::AlignLeft,		{ db->ascentsTable->peakIDColumn },			db->peaksTable->nameColumn)),
			peakHeightColumn		(new const ReferenceCompositeColumn		(tr("Height"),			Qt::AlignRight,		{ db->ascentsTable->peakIDColumn },			db->peaksTable->heightColumn)),
			regionColumn			(new const ReferenceCompositeColumn		(tr("Region"),			Qt::AlignLeft,		{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn},			db->regionsTable->nameColumn)),
			rangeColumn				(new const ReferenceCompositeColumn		(tr("Mountain range"),	Qt::AlignLeft,		{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn,				db->regionsTable->rangeIDColumn},		db->rangesTable->nameColumn)),
			countryColumn			(new const ReferenceCompositeColumn		(tr("Country"),			Qt::AlignLeft,		{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn,				db->regionsTable->countryIDColumn},		db->countriesTable->nameColumn)),
			continentColumn			(new const ReferenceCompositeColumn		(tr("Continent"),		Qt::AlignLeft,		{ db->ascentsTable->peakIDColumn,			db->peaksTable->regionIDColumn,				db->regionsTable->rangeIDColumn},		db->rangesTable->continentColumn,		&Range::continentNames)),
			volcanoColumn			(new const ReferenceCompositeColumn		(tr("Volcano"),			Qt::AlignCenter,	{ db->ascentsTable->peakIDColumn },			db->peaksTable->volcanoColumn)),
			elevationGainColumn		(new const DirectCompositeColumn		(tr("Elev. gain"),		Qt::AlignRight,		db->ascentsTable->elevationGainColumn)),
			hikeKindColumn			(new const DirectCompositeColumn		(tr("Hike kind"),		Qt::AlignLeft,		db->ascentsTable->hikeKindColumn,			&Ascent::hikeKindNames)),
			traverseColumn			(new const DirectCompositeColumn		(tr("Traverse"),		Qt::AlignCenter,	db->ascentsTable->traverseColumn)),
			difficultyGradeColumn	(new const DependentEnumCompositeColumn	(tr("Difficulty"),							db->ascentsTable->difficultySystemColumn,	db->ascentsTable->difficultyGradeColumn,	&Ascent::difficultyNames)),
			hikersColumn			(new const FoldCompositeColumn			(tr("Hikers"),			List,				{ {db->ascentsTable->primaryKeyColumn,		db->participatedTable->ascentIDColumn},		{db->participatedTable->hikerIDColumn,	db->hikersTable->primaryKeyColumn} },	db->hikersTable->nameColumn)),
			tripColumn				(new const ReferenceCompositeColumn		(tr("Trip"),			Qt::AlignLeft,		{ db->ascentsTable->tripIDColumn },			db->tripsTable->nameColumn))
	{
		addColumn(dateColumn);
		addColumn(titleColumn);
		addColumn(peakColumn);
		addColumn(peakHeightColumn);
		addColumn(regionColumn);
		addColumn(rangeColumn);
		addColumn(countryColumn);
		addColumn(continentColumn);
		addColumn(volcanoColumn);
		addColumn(elevationGainColumn);
		addColumn(hikeKindColumn);
		addColumn(traverseColumn);
		addColumn(difficultyGradeColumn);
		addColumn(hikersColumn);
		addColumn(tripColumn);
	}
};



#endif // COMP_ASCENTS_TABLE_H
