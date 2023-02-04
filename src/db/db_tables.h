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
#include <QTranslator>



class AscentsTable : public NormalTable {
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
	
	AscentsTable(Column* peakIDColumn, Column* tripIDColumn) :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//									name							uiName							type		nullable	primaryKey	foreignKey		inTable
			titleColumn				(new Column(QString("title"),				QObject::tr("Title"),			varchar,	true,		false,		nullptr,		this)),
			peakIDColumn			(new Column(QString("peakID"),				QString(),						integer,	true,		false,		peakIDColumn,	this)),
			dateColumn				(new Column(QString("date"),				QObject::tr("Date"),			date,		true,		false,		nullptr,		this)),
			peakOnDayColumn			(new Column(QString("peakOnDay"),			QObject::tr("Peak of the day"),	integer,	false,		false,		nullptr,		this)),
			timeColumn				(new Column(QString("time"),				QObject::tr("Time"),			time_,		true,		false,		nullptr,		this)),
			hikeKindColumn			(new Column(QString("hikeKind"),			QObject::tr("Kind of hike"),	integer,	false,		false,		nullptr,		this)),
			traverseColumn			(new Column(QString("traverse"),			QObject::tr("Traverse"),		bit,		false,		false,		nullptr,		this)),
			difficultySystemColumn	(new Column(QString("difficultySystem"),	QObject::tr("Diff. system"),	integer,	true,		false,		nullptr,		this)),
			difficultyGradeColumn	(new Column(QString("difficultyGrade"),		QObject::tr("Diff. grade"),		integer,	true,		false,		nullptr,		this)),
			tripIDColumn			(new Column(QString("tripID"),				QString(),						integer,	true,		false,		tripIDColumn,	this)),
			notesColumn				(new Column(QString("notes"),				QObject::tr("Notes"),			varchar,	true,		false,		nullptr,		this))
	{
		addColumn(titleColumn);
		addColumn(peakIDColumn);
		addColumn(dateColumn);
		addColumn(peakOnDayColumn);
		addColumn(timeColumn);
		addColumn(hikeKindColumn);
		addColumn(traverseColumn);
		addColumn(difficultySystemColumn);
		addColumn(difficultyGradeColumn	);
		addColumn(tripIDColumn);
		addColumn(notesColumn);
	}
	
	void addRow(Ascent* ascent);
};



class PeaksTable : public NormalTable {
public:
	Column* nameColumn;
	Column* heightColumn;
	Column* volcanoColumn;
	Column* regionIDColumn;
	Column* mapsLinkColumn;
	Column* earthLinkColumn;
	Column* wikiLinkColumn;
	
	PeaksTable(Column* regionIDColumn) :
			NormalTable(QString("Peaks"), QString("peak"), QObject::tr("Peaks")),
			//							name					uiName								type		nullable	primaryKey	foreignKey		inTable
			nameColumn		(new Column(QString("name"),		QObject::tr("Name"),				varchar,	false,		false,		nullptr,		this)),
			heightColumn	(new Column(QString("height"),		QObject::tr("Height"),				integer,	true,		false,		nullptr,		this)),
			volcanoColumn	(new Column(QString("volcano"),		QObject::tr("Volcano"),				bit,		false,		false,		nullptr,		this)),
			regionIDColumn	(new Column(QString("regionID"),	QString(),							integer,	true,		false,		regionIDColumn,	this)),
			mapsLinkColumn	(new Column(QString("mapsLink"),	QObject::tr("Google Maps link"),	varchar,	true,		false,		nullptr,		this)),
			earthLinkColumn	(new Column(QString("earthLink"),	QObject::tr("Google Earth link"),	varchar,	true,		false,		nullptr,		this)),
			wikiLinkColumn	(new Column(QString("wikiLink"),	QObject::tr("Wikipedia link"),		varchar,	true,		false,		nullptr,		this))
	{
		addColumn(nameColumn);
		addColumn(heightColumn);
		addColumn(volcanoColumn);
		addColumn(regionIDColumn);
		addColumn(mapsLinkColumn);
		addColumn(earthLinkColumn);
		addColumn(wikiLinkColumn);
	}
	
	void addRow(Peak* peak);
};



class TripsTable : public NormalTable {
public:
	Column* nameColumn;
	Column* startDateColumn;
	Column* endDateColumn;
	Column* notesColumn;
	
	TripsTable() :
			NormalTable(QString("Trips"), QString("trip"), QObject::tr("Trips")),
			//							name					uiName						type		nullable	primaryKey	foreignKey	inTable
			nameColumn		(new Column(QString("name"),		QObject::tr("Name"),		varchar,	true,		false,		nullptr,	this)),
			startDateColumn	(new Column(QString("startDate"),	QObject::tr("Start date"),	date,		true,		false,		nullptr,	this)),
			endDateColumn	(new Column(QString("endDate"),		QObject::tr("End date"),	date,		true,		false,		nullptr,	this)),
			notesColumn		(new Column(QString("notes"),		QObject::tr("Notes"),		varchar,	true,		false,		nullptr,	this))
	{
		addColumn(nameColumn);
		addColumn(startDateColumn);
		addColumn(endDateColumn);
		addColumn(notesColumn);
	}
	
	void addRow(Trip* trip);
};



class HikersTable : public NormalTable {
public:
	Column* nameColumn;
	
	HikersTable() :
			NormalTable(QString("Hikers"), QString("hiker"), QObject::tr("Hikers")),
			//						name				uiName					type		nullable	primaryKey	foreignKey	inTable
			nameColumn	(new Column(QString("name"),	QObject::tr("Name"),	varchar,	false,		false,		nullptr,	this))
	{
		addColumn(nameColumn);
	}
	
	void addRow(Hiker* hiker);
};



class RegionsTable : public NormalTable {
public:
	Column* nameColumn;
	Column* rangeIDColumn;
	Column* countryIDColumn;
	
	RegionsTable(Column* rangeIDColumn, Column* countryIDColumn) :
			NormalTable(QString("Regions"), QString("region"), QObject::tr("Regions")),
			//							name					uiName					type		nullable	primaryKey	foreignKey			inTable
			nameColumn		(new Column(QString("name"),		QObject::tr("Name"),	varchar,	false,		false,		nullptr,			this)),
			rangeIDColumn	(new Column(QString("rangeID"),		QString(),				integer,	true,		false,		rangeIDColumn,		this)),
			countryIDColumn	(new Column(QString("countryID"),	QString(),				integer,	true,		false,		countryIDColumn,	this))
	{
		addColumn(nameColumn);
		addColumn(rangeIDColumn);
		addColumn(countryIDColumn);
	}
	
	void addRow(Region* region);
};



class RangesTable : public NormalTable {
public:
	Column* nameColumn;
	Column* continentColumn;
	
	RangesTable() :
			NormalTable(QString("Ranges"), QString("range"), QObject::tr("Mountain ranges")),
			//							name					uiName						type		nullable	primaryKey	foreignKey	inTable
			nameColumn		(new Column(QString("name"),		QObject::tr("Name"),		varchar,	false,		false,		nullptr,	this)),
			continentColumn	(new Column(QString("continent"),	QObject::tr("Continent"),	integer,	true,		false,		nullptr,	this))
	{
		addColumn(nameColumn);
		addColumn(continentColumn);
	}
	
	void addRow(Range* range);
};



class CountriesTable : public NormalTable {
public:
	Column* nameColumn;
	
	CountriesTable() :
			NormalTable(QString("Countries"), QString("country"), QObject::tr("Countries")),
			//						name				uiName					type		nullable	primaryKey	foreignKey	inTable
			nameColumn	(new Column(QString("name"),	QObject::tr("Name"),	varchar,	false,		false,		nullptr,	this))
	{
		addColumn(nameColumn);
	}
	
	void addRow(Country* country);
};



class PhotosTable : public NormalTable {
public:
	Column* ascentIDColumn;
	Column* indexColumn;
	Column* filepathColumn;
	
	PhotosTable(Column* ascentIDColumn) :
			NormalTable(QString("Photos"), QString("photo"), QString()),
			//				name					uiName						type		nullable	primaryKey	foreignKey		inTable
			ascentIDColumn	(new Column(QString("ascentID"),	QString(),					integer,	true,		false,		ascentIDColumn,	this)),
			indexColumn		(new Column(QString("photoIndex"),	QObject::tr("Index"),		integer,	true,		false,		nullptr,		this)),
			filepathColumn	(new Column(QString("filepath"),	QObject::tr("File path"),	varchar,	true,		false,		nullptr,		this))
	{
		addColumn(ascentIDColumn);
		addColumn(indexColumn);
		addColumn(filepathColumn);
	}
	
	void addRows(Ascent* ascent);
};



class ParticipatedTable : public AssociativeTable {
public:
	ParticipatedTable(Column* ascentIDColumn, Column* hikerIDColumn) :
			AssociativeTable(QString("Participated"), QString(),
				//			name					uiName		type		nullable	primaryKey	foreignKey		inTable
				new Column	(QString("ascentID"),	QString(),	integer,	false,		true,		ascentIDColumn,	this),
				new Column	(QString("hikerID"),	QString(),	integer,	false,		true,		hikerIDColumn,	this)
			)
	{}
	
	void addRows(Ascent* ascent);
};



#endif // DB_TABLES_H
