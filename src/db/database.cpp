#include "database.h"

#include "src/db/db_error.h"
#include "src/db/test_db.h"
#include "src/main/main_window.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>



Database::Database(MainWindow* parent) :
		tables(QList<Table*>()),
		mainWindowStatusBar(nullptr)
{
	tripsTable			= new TripsTable();
	hikersTable			= new HikersTable();
	rangesTable			= new RangesTable();
	countriesTable		= new CountriesTable();
	regionsTable		= new RegionsTable(rangesTable->getPrimaryKeyColumn(), countriesTable->getPrimaryKeyColumn());
	peaksTable			= new PeaksTable(regionsTable->getPrimaryKeyColumn());
	ascentsTable		= new AscentsTable(peaksTable->getPrimaryKeyColumn(), tripsTable->getPrimaryKeyColumn());
	photosTable			= new PhotosTable(ascentsTable->getPrimaryKeyColumn());
	participatedTable	= new ParticipatedTable(ascentsTable->getPrimaryKeyColumn(), hikersTable->getPrimaryKeyColumn());
	
	QSqlError initError = initDB();
	if (initError.type() != QSqlError::NoError) {
		displayError(parent, initError);
	}
	
	tables.append(tripsTable);
	tables.append(hikersTable);
	tables.append(rangesTable);
	tables.append(countriesTable);
	tables.append(regionsTable);
	tables.append(peaksTable);
	tables.append(ascentsTable);
	tables.append(photosTable);
	tables.append(participatedTable);
	
	for (auto iter = tables.constBegin(); iter != tables.constEnd(); iter++) {
		(*iter)->initBuffer(parent);
	}
}

Database::~Database() {
	qDeleteAll(getTableList());
}



QList<Table*> Database::getTableList() const
{
	return QList<Table*>(tables);
}



Ascent* Database::getAscent(ValidItemID ascentID) const
{
	return getAscentAt(ascentsTable->getBufferIndexForPrimaryKey(ascentID));
}

Peak* Database::getPeak(ValidItemID peakID) const
{
	return getPeakAt(peaksTable->getBufferIndexForPrimaryKey(peakID));
}

Trip* Database::getTrip(ValidItemID tripID) const
{
	return getTripAt(tripsTable->getBufferIndexForPrimaryKey(tripID));
}

Hiker* Database::getHiker(ValidItemID hikerID) const
{
	return getHikerAt(hikersTable->getBufferIndexForPrimaryKey(hikerID));
}

Region* Database::getRegion(ValidItemID regionID) const
{
	return getRegionAt(regionsTable->getBufferIndexForPrimaryKey(regionID));
}

Range* Database::getRange(ValidItemID rangeID) const
{
	return getRangeAt(rangesTable->getBufferIndexForPrimaryKey(rangeID));
}

Country* Database::getCountry(ValidItemID countryID) const
{
	return getCountryAt(countriesTable->getBufferIndexForPrimaryKey(countryID));
}



Ascent* Database::getAscentAt(int rowIndex) const
{
	const QList<QVariant>* row = ascentsTable->getBufferRow(rowIndex);
	assert(row->size() == ascentsTable->getNumberOfColumns());
	
	ValidItemID	ascentID			= row->at(ascentsTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		title				= row->at(ascentsTable->titleColumn->getIndex()).toString();
	ItemID		peakID				= row->at(ascentsTable->peakIDColumn->getIndex()).toInt();
	QDate		date				= row->at(ascentsTable->dateColumn->getIndex()).toDate();
	int			perDayIndex			= row->at(ascentsTable->peakOnDayColumn->getIndex()).toInt();
	QTime		time				= row->at(ascentsTable->timeColumn->getIndex()).toTime();
	int			elevationGain		= row->at(ascentsTable->elevationGainColumn->getIndex()).toInt();
	int			hikeKind			= row->at(ascentsTable->hikeKindColumn->getIndex()).toInt();
	bool		traverse			= row->at(ascentsTable->traverseColumn->getIndex()).toBool();
	int			difficultySystem	= row->at(ascentsTable->difficultySystemColumn->getIndex()).toInt();
	int			difficultyGrade		= row->at(ascentsTable->difficultyGradeColumn->getIndex()).toInt();
	ItemID		tripID				= row->at(ascentsTable->tripIDColumn->getIndex()).toInt();
	QString		description			= row->at(ascentsTable->descriptionColumn->getIndex()).toString();
	
	QSet<ValidItemID>	hikerIDs	= participatedTable->getMatchingEntries(participatedTable->ascentIDColumn, ascentID);
	QStringList			photos		= photosTable->getPhotosForAscent(ascentID);
	
	setStatusBarMessage(QString("Successfully retrieved ascent with ascentID=%1").arg(ascentID.get()));
	return new Ascent(ascentID, title, peakID, date, perDayIndex, time, elevationGain, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, hikerIDs, photos, description);
}

Peak* Database::getPeakAt(int rowIndex) const
{
	const QList<QVariant>* row = peaksTable->getBufferRow(rowIndex);
	assert(row->size() == peaksTable->getNumberOfColumns());
	
	ValidItemID	peakID		= row->at(peaksTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		name		= row->at(peaksTable->nameColumn->getIndex()).toString();
	int			height		= row->at(peaksTable->heightColumn->getIndex()).toInt();
	bool		volcano		= row->at(peaksTable->volcanoColumn->getIndex()).toBool();
	int			regionID	= row->at(peaksTable->regionIDColumn->getIndex()).toInt();
	QString		mapsLink	= row->at(peaksTable->mapsLinkColumn->getIndex()).toString();
	QString		earthLink	= row->at(peaksTable->earthLinkColumn->getIndex()).toString();
	QString		wikiLink	= row->at(peaksTable->wikiLinkColumn->getIndex()).toString();
	
//	setStatusBarMessage(QString("Successfully retrieved peak with peakID=%1").arg(peakID.get()));
	return new Peak(peakID, name, height, volcano, regionID, mapsLink, earthLink, wikiLink);
}

Trip* Database::getTripAt(int rowIndex) const
{
	const QList<QVariant>* row = tripsTable->getBufferRow(rowIndex);
	assert(row->size() == tripsTable->getNumberOfColumns());
	
	ValidItemID	tripID		= row->at(tripsTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		name		= row->at(tripsTable->nameColumn->getIndex()).toString();
	QDate		startDate	= row->at(tripsTable->startDateColumn->getIndex()).toDate();
	QDate		endDate		= row->at(tripsTable->endDateColumn->getIndex()).toDate();
	QString		description	= row->at(tripsTable->descriptionColumn->getIndex()).toString();
	
//	setStatusBarMessage(QString("Successfully retrieved trip with tripID=%1").arg(tripID.get()));
	return new Trip(tripID, name, startDate, endDate, description);
}

Hiker* Database::getHikerAt(int rowIndex) const
{
	const QList<QVariant>* row = hikersTable->getBufferRow(rowIndex);
	assert(row->size() == hikersTable->getNumberOfColumns());
	
	ValidItemID	hikerID	= row->at(hikersTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		name	= row->at(hikersTable->nameColumn->getIndex()).toString();
	
//	setStatusBarMessage(QString("Successfully retrieved hiker with hikerID=%1").arg(hikerID.get()));
	return new Hiker(hikerID, name);
}

Region* Database::getRegionAt(int rowIndex) const
{
	const QList<QVariant>* row = regionsTable->getBufferRow(rowIndex);
	assert(row->size() == regionsTable->getNumberOfColumns());
	
	ValidItemID	regionID	= row->at(regionsTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		name		= row->at(regionsTable->nameColumn->getIndex()).toString();
	int			rangeID		= row->at(regionsTable->rangeIDColumn->getIndex()).toInt();
	int			countryID	= row->at(regionsTable->countryIDColumn->getIndex()).toInt();
	
//	setStatusBarMessage(QString("Successfully retrieved region with regionID=%1").arg(regionID.get()));
	return new Region(regionID, name, rangeID, countryID);
}

Range* Database::getRangeAt(int rowIndex) const
{
	const QList<QVariant>* row = rangesTable->getBufferRow(rowIndex);
	assert(row->size() == rangesTable->getNumberOfColumns());
	
	ValidItemID	rangeID		= row->at(rangesTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		name		= row->at(rangesTable->nameColumn->getIndex()).toString();
	int			continent	= row->at(rangesTable->continentColumn->getIndex()).toInt();
	
//	setStatusBarMessage(QString("Successfully retrieved range with rangeID=%1").arg(rangeID.get()));
	return new Range(rangeID, name, continent);
}

Country* Database::getCountryAt(int rowIndex) const
{
	const QList<QVariant>* row = countriesTable->getBufferRow(rowIndex);
	assert(row->size() == countriesTable->getNumberOfColumns());
	
	ValidItemID	countryID	= row->at(countriesTable->getPrimaryKeyColumn()->getIndex()).toInt();
	QString		name		= row->at(countriesTable->nameColumn->getIndex()).toString();
	
//	setStatusBarMessage(QString("Successfully retrieved country with countryID=%1").arg(countryID.get()));
	return new Country(countryID, name);
}



QList<WhatIfDeleteResult> Database::whatIf_removeRow(NormalTable* table, ValidItemID primaryKey)
{
	return Database::removeRow_referenceSearch(nullptr, true, table, primaryKey);
}

void Database::removeRow(QWidget* parent, NormalTable* table, ValidItemID primaryKey)
{
	Database::removeRow_referenceSearch(parent, false, table, primaryKey);
	
	table->removeRow(parent, primaryKey);
}

QList<WhatIfDeleteResult> Database::removeRow_referenceSearch(QWidget* parent, bool searchNotExecute, NormalTable* table, ValidItemID primaryKey)
{
	const Column* primaryKeyColumn = table->getPrimaryKeyColumn();
	QList<WhatIfDeleteResult> result = QList<WhatIfDeleteResult>();
	for (auto iter = tables.constBegin(); iter != tables.constEnd(); iter++) {
		
		// Look for references in associative table
		if ((*iter)->isAssociative) {
			AssociativeTable* candidateTable = (AssociativeTable*) *iter;
			
			const Column* matchingColumn = candidateTable->getOwnColumnReferencing(primaryKeyColumn);
			if (!matchingColumn) continue;
			
			// WHAT IF
			if (searchNotExecute) {
				int numAffectedRowIndices = candidateTable->getNumberOfMatchingRows(matchingColumn, primaryKey);
				if (numAffectedRowIndices > 0) {
					const NormalTable* itemTable = candidateTable->traverseAssociativeRelation(primaryKeyColumn);
					result.append(WhatIfDeleteResult(candidateTable, itemTable, numAffectedRowIndices));
				}
			}
			// EXECUTE
			else {
				candidateTable->removeMatchingRows(parent, matchingColumn, primaryKey);
			}
		}
		
		// Look for references in normal table
		else {
			NormalTable* candidateTable = (NormalTable*) *iter;
			
			QSet<int> affectedRowIndices = QSet<int>();
			QList<QPair<const Column*, QList<int>>> affectedCells = QList<QPair<const Column*, QList<int>>>();
			
			for (const Column* otherTableColumn : candidateTable->getColumnList()) {
				if (otherTableColumn->getReferencedForeignColumn() != primaryKeyColumn) continue;
				
				QList<int> rowIndexList = candidateTable->getMatchingBufferRowIndices(otherTableColumn, primaryKey.get());
				QSet<int> rowIndexSet = QSet<int>();
				for (int index : rowIndexList) rowIndexSet.insert(index);
				
				affectedRowIndices.unite(rowIndexSet);
				affectedCells.append({ otherTableColumn, rowIndexList });
			}
			
			// WHAT IF
			if (searchNotExecute) {
				if (!affectedCells.isEmpty()) {
					result.append(WhatIfDeleteResult(candidateTable, candidateTable, affectedRowIndices.size()));
				}
			}
			// EXECUTE
			else {
				for (auto iter = affectedCells.constBegin(); iter != affectedCells.constEnd(); iter++) {
					const Column* column = (*iter).first;
					QList<int> rowIndices = (*iter).second;
					NormalTable* table = (NormalTable*) column->table;
					primaryKeyColumn = table->getPrimaryKeyColumn();
					
					for (int rowIndex : rowIndices) {
						ValidItemID primaryKey = table->getBufferRow(rowIndex)->at(primaryKeyColumn->getIndex()).toInt();
						// Remove single instance of reference to the key about to be removed
						candidateTable->updateCell(parent, primaryKey, column, ItemID().asQVariant());
					}
				}
			}
		}
		
	}
	return result;
}

void Database::removeRow(QWidget* parent, AssociativeTable* table, ValidItemID primaryKey1, ValidItemID primaryKey2)
{
	// TODO
}



void Database::setStatusBar(QStatusBar* mainWindowStatusBar)
{
	this->mainWindowStatusBar = mainWindowStatusBar;
}

void Database::setStatusBarMessage(QString content) const
{
	if (!mainWindowStatusBar) {
		qDebug() << "class Database: Tried to show message on status bar but status bar has not been set";
		return;
	}
	return mainWindowStatusBar->showMessage(content);
}



int Database::getIntFromRecord(QWidget* parent, QSqlQuery& query, QString& queryString, int entryInd) const
{
	assert(entryInd >= 0);
	QVariant variantValue = query.value(entryInd);
	if (!variantValue.isValid())
		displayError(parent, "Received invalid QVariant from query", queryString);
	bool conversionOk;
	int intValue = variantValue.toInt(&conversionOk);
	if (!conversionOk)
		displayError(parent, "Conversion to int failed for result from query", queryString);
	return intValue;
}

QString Database::getStringFromRecord(QWidget* parent, QSqlQuery& query, QString& queryString, int entryInd) const
{
	QVariant variantValue = query.value(entryInd);
	if (!variantValue.isValid())
		displayError(parent, "Received invalid QVariant from query", queryString);
	QString stringValue = variantValue.toString();
	return stringValue;
}





WhatIfDeleteResult::WhatIfDeleteResult(const Table* affectedTable, const NormalTable* itemTable, int numAffectedRowIndices) :
		affectedTable(affectedTable),
		itemTable(itemTable),
		numAffectedRowIndices(numAffectedRowIndices)
{}
