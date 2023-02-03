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
	Column titleColumn;
	Column peakIDColumn;
	Column dateColumn;
	Column peakOnDayColumn;
	Column timeColumn;
	Column hikeKindColumn;
	Column traverseColumn;
	Column difficultySystemColumn;
	Column difficultyGradeColumn;
	Column tripIDColumn;
	Column notesColumn;
	
	AscentsTable(Column* peakIDColumn, Column* tripIDColumn) :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//						name							uiName							type		nullable	primaryKey	foreignKey		inTable
			titleColumn				(QString("title"),				QObject::tr("Title"),			varchar,	true,		false,		nullptr,		this),
			peakIDColumn			(QString("peakID"),				QString(),						integer,	true,		false,		peakIDColumn,	this),
			dateColumn				(QString("date"),				QObject::tr("Date"),			date,		true,		false,		nullptr,		this),
			peakOnDayColumn			(QString("peakOnDay"),			QObject::tr("Peak of the day"),	integer,	false,		false,		nullptr,		this),
			timeColumn				(QString("time"),				QObject::tr("Time"),			time_,		true,		false,		nullptr,		this),
			hikeKindColumn			(QString("hikeKind"),			QObject::tr("Kind of hike"),	integer,	false,		false,		nullptr,		this),
			traverseColumn			(QString("traverse"),			QObject::tr("Traverse"),		bit,		false,		false,		nullptr,		this),
			difficultySystemColumn	(QString("difficultySystem"),	QObject::tr("Diff. system"),	integer,	true,		false,		nullptr,		this),
			difficultyGradeColumn	(QString("difficultyGrade"),	QObject::tr("Diff. grade"),		integer,	true,		false,		nullptr,		this),
			tripIDColumn			(QString("tripID"),				QString(),						integer,	true,		false,		tripIDColumn,	this),
			notesColumn				(QString("notes"),				QObject::tr("Notes"),			varchar,	true,		false,		nullptr,		this)
	{}
	
	void addRow(Ascent* ascent);
};



class PeaksTable : public NormalTable {
public:
	Column nameColumn;
	Column heightColumn;
	Column volcanoColumn;
	Column regionIDColumn;
	Column mapsLinkColumn;
	Column earthLinkColumn;
	Column wikiLinkColumn;
	
	PeaksTable(Column* regionIDColumn) :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//				name					uiName								type		nullable	primaryKey	foreignKey		inTable
			nameColumn		(QString("name"),		QObject::tr("Name"),				varchar,	false,		false,		nullptr,		this),
			heightColumn	(QString("height"),		QObject::tr("Height"),				integer,	true,		false,		nullptr,		this),
			volcanoColumn	(QString("volcano"),	QObject::tr("Volcano"),				bit,		false,		false,		nullptr,		this),
			regionIDColumn	(QString("regionID"),	QString(),							integer,	true,		false,		regionIDColumn,	this),
			mapsLinkColumn	(QString("mapsLink"),	QObject::tr("Google Maps link"),	varchar,	true,		false,		nullptr,		this),
			earthLinkColumn	(QString("earthLink"),	QObject::tr("Google Earth link"),	varchar,	true,		false,		nullptr,		this),
			wikiLinkColumn	(QString("wikiLink"),	QObject::tr("Wikipedia link"),		varchar,	true,		false,		nullptr,		this)
	{}
	
	void addRow(Peak* peak);
};



class TripsTable : public NormalTable {
public:
	Column nameColumn;
	Column startDateColumn;
	Column endDateColumn;
	Column notesColumn;
	
	TripsTable() :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//				name					uiName						type		nullable	primaryKey	foreignKey	inTable
			nameColumn		(QString("name"),		QObject::tr("Name"),		varchar,	true,		false,		nullptr,	this),
			startDateColumn	(QString("startDate"),	QObject::tr("Start date"),	date,		true,		false,		nullptr,	this),
			endDateColumn	(QString("endDate"),	QObject::tr("End date"),	date,		true,		false,		nullptr,	this),
			notesColumn		(QString("notes"),		QObject::tr("Notes"),		varchar,	true,		false,		nullptr,	this)
	{}
	
	void addRow(Trip* trip);
};



class HikersTable : public NormalTable {
public:
	Column nameColumn;
	
	HikersTable() :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//			name				uiName					type		nullable	primaryKey	foreignKey	inTable
			nameColumn	(QString("name"),	QObject::tr("Name"),	varchar,	false,		false,		nullptr,	this)
	{}
	
	void addRow(Hiker* hiker);
};



class RegionsTable : public NormalTable {
public:
	Column nameColumn;
	Column rangeIDColumn;
	Column countryIDColumn;
	
	RegionsTable(Column* rangeIDColumn, Column* countryIDColumn) :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//				name					uiName					type		nullable	primaryKey	foreignKey			inTable
			nameColumn		(QString("name"),		QObject::tr("Name"),	varchar,	false,		false,		nullptr,			this),
			rangeIDColumn	(QString("rangeID"),	QString(),				integer,	true,		false,		rangeIDColumn,		this),
			countryIDColumn	(QString("countryID"),	QString(),				integer,	true,		false,		countryIDColumn,	this)
	{}
	
	void addRow(Region* region);
};



class RangesTable : public NormalTable {
public:
	Column nameColumn;
	Column continentColumn;
	
	RangesTable() :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//				name					uiName						type		nullable	primaryKey	foreignKey	inTable
			nameColumn		(QString("name"),		QObject::tr("Name"),		varchar,	false,		false,		nullptr,	this),
			continentColumn	(QString("continent"),	QObject::tr("Continent"),	integer,	true,		false,		nullptr,	this)
	{}
	
	void addRow(Range* range);
};



class CountriesTable : public NormalTable {
public:
	Column nameColumn;
	
	CountriesTable() :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//			name				uiName					type		nullable	primaryKey	foreignKey	inTable
			nameColumn	(QString("name"),	QObject::tr("Name"),	varchar,	false,		false,		nullptr,	this)
	{}
	
	void addRow(Country* country);
};



class PhotosTable : public NormalTable {
public:
	Column ascentIDColumn;
	Column indexColumn;
	Column filepathColumn;
	
	PhotosTable(Column* ascentIDColumn) :
			NormalTable(QString("Ascents"), QString("ascent"), QObject::tr("Ascents")),
			//				name					uiName						type		nullable	primaryKey	foreignKey		inTable
			ascentIDColumn	(QString("ascentID"),	QString(),					integer,	true,		false,		ascentIDColumn,	this),
			indexColumn		(QString("photoIndex"),	QObject::tr("Index"),		integer,	true,		false,		nullptr,		this),
			filepathColumn	(QString("filepath"),	QObject::tr("File path"),	varchar,	true,		false,		nullptr,		this)
	{}
	
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
