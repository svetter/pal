#ifndef TEST_DB_H
#define TEST_DB_H

#include <QtSql>



const auto CREATE_ASCENTS		= "CREATE TABLE Ascents(ascentID INTEGER PRIMARY KEY, title NVARCHAR, peakID INTEGER REFERENCES Peaks(peakID), date DATE, peakOnDay INTEGER NOT NULL, time TIME, hikeKind INT NOT NULL, traverse BINARY NOT NULL, difficultySystem INTEGER, difficultyGrade INTEGER, tripID INTEGER REFERENCES Trips(tripID), notes NVARCHAR)";
const auto CREATE_PEAKS			= "CREATE TABLE Peaks(peakID INTEGER PRIMARY KEY, name NVARCHAR NOT NULL, height INTEGER, volcano BINARY NOT NULL, regionID INTEGER REFERENCES Regions(regionID), mapsLink NVARCHAR, earthLink NVARCHAR, wikiLink NVARCHAR)";
const auto CREATE_TRIPS			= "CREATE TABLE Trips(tripID INTEGER PRIMARY KEY, name NVARCHAR NOT NULL, startDate DATE, endDate DATE, notes NVARCHAR)";
const auto CREATE_HIKERS		= "CREATE TABLE Hikers(hikerID INTEGER PRIMARY KEY, name NVARCHAR NOT NULL)";
const auto CREATE_REGIONS		= "CREATE TABLE Regions(regionID INTEGER PRIMARY KEY, name NVARCHAR NOT NULL, rangeID INTEGER REFERENCES Ranges(rangeID), countryID INTEGER REFERENCES Countries(countryID))";
const auto CREATE_RANGES		= "CREATE TABLE Ranges(rangeID INTEGER PRIMARY KEY, name NVARCHAR NOT NULL, continent INT)";
const auto CREATE_COUNTRIES		= "CREATE TABLE Countries(countryID INTEGER PRIMARY KEY, name NVARCHAR NOT NULL)";
const auto CREATE_PHOTOS		= "CREATE TABLE Photos(photoID INTEGER PRIMARY KEY, ascentID INTEGER REFERENCES Ascents(ascentID), localIndex INTEGER NOT NULL, filepath NVARCHAR NOT NULL)";
const auto CREATE_PARTICIPATED	= "CREATE TABLE Participated(ascentID INTEGER NOT NULL, hikerID INTEGER NOT NULL, CONSTRAINT participatedPK PRIMARY KEY (ascentID, hikerID))";

const auto INSERT_ASCENT		= "INSERT INTO Ascents(title, peakID, date, peakOnDay, time, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, notes) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
const auto INSERT_PEAK			= "INSERT INTO Peaks(name, height, volcano, regionID, mapsLink, earthLink, wikiLink) values(?, ?, ?, ?, ?, ?, ?)";
const auto INSERT_TRIP			= "INSERT INTO Trips(name, startDate, endDate, notes) values(?, ?, ?, ?)";
const auto INSERT_HIKER			= "INSERT INTO Hikers(name) values(?)";
const auto INSERT_REGION		= "INSERT INTO Regions(name, rangeID, countryID) values(?, ?, ?)";
const auto INSERT_RANGE			= "INSERT INTO Ranges(name, continent) values(?, ?)";
const auto INSERT_COUNTRY		= "INSERT INTO Countries(name) values(?)";
const auto INSERT_PHOTO			= "INSERT INTO Photos(ascentID, index, filepath) values(?, ?, ?)";
const auto INSERT_PARTICIPATED	= "INSERT INTO Participated(ascentID, hikerID) values(?, ?)";



int addAscent(QSqlQuery& q, const QString& title, int peakID, const QDate& date, int peakOnDay, const QTime& time, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, int tripID, const QString& notes)
{
	q.addBindValue(title);
	q.addBindValue(peakID);
	q.addBindValue(date);
	q.addBindValue(peakOnDay);
	q.addBindValue(time);
	q.addBindValue(hikeKind);
	q.addBindValue(traverse);
	q.addBindValue(difficultySystem);
	q.addBindValue(difficultyGrade);
	q.addBindValue(tripID);
	q.addBindValue(notes);
	q.exec();
	return q.lastInsertId().toInt();
}

int addPeak(QSqlQuery& q, const QString& name, int height, bool volcano, int regionID, const QString& mapsLink, const QString& earthLink, const QString& wikiLink)
{
	q.addBindValue(name);
	q.addBindValue(height);
	q.addBindValue(volcano);
	q.addBindValue(regionID);
	q.addBindValue(mapsLink);
	q.addBindValue(earthLink);
	q.addBindValue(wikiLink);
	q.exec();
	return q.lastInsertId().toInt();
}

int addTrip(QSqlQuery& q, const QString& name, const QDate& startDate, const QDate& endDate, const QString& notes)
{
	q.addBindValue(name);
	q.addBindValue(startDate);
	q.addBindValue(endDate);
	q.addBindValue(notes);
	q.exec();
	return q.lastInsertId().toInt();
}

int addHiker(QSqlQuery& q, const QString& name)
{
	q.addBindValue(name);
	q.exec();
	return q.lastInsertId().toInt();
}

int addCountry(QSqlQuery& q, const QString& name)
{
	q.addBindValue(name);
	q.exec();
	return q.lastInsertId().toInt();
}

int addRange(QSqlQuery& q, const QString& name, int continent)
{
	q.addBindValue(name);
	q.addBindValue(continent);
	q.exec();
	return q.lastInsertId().toInt();
}

int addRegion(QSqlQuery& q, const QString& name, int rangeID, int countryID)
{
	q.addBindValue(name);
	q.addBindValue(rangeID);
	q.addBindValue(countryID);
	q.exec();
	return q.lastInsertId().toInt();
}

int addPhoto(QSqlQuery& q, int ascentID, int index, const QString& filepath)
{
	q.addBindValue(ascentID);
	q.addBindValue(index);
	q.addBindValue(filepath);
	q.exec();
	return q.lastInsertId().toInt();
}

int addParticipated(QSqlQuery& q, int ascentID, int hikerID)
{
	q.addBindValue(ascentID);
	q.addBindValue(hikerID);
	q.exec();
	return q.lastInsertId().toInt();
}



QSqlError initDB()
{
	QSqlDatabase sql = QSqlDatabase::addDatabase("QSQLITE");
	sql.setDatabaseName(":memory:");
	
	if (!sql.open())
		qDebug() << sql.lastError();
	
	QSqlQuery q;
	if (!q.exec(CREATE_ASCENTS)
			|| !q.exec(CREATE_PEAKS)
			|| !q.exec(CREATE_TRIPS)
			|| !q.exec(CREATE_HIKERS)
			|| !q.exec(CREATE_REGIONS)
			|| !q.exec(CREATE_RANGES)
			|| !q.exec(CREATE_COUNTRIES)
			|| !q.exec(CREATE_PHOTOS)
			|| !q.exec(CREATE_PARTICIPATED)) {
		return q.lastError();
	}
	
	if (!q.prepare(INSERT_COUNTRY)) return q.lastError();
	int germanyID	= addCountry(q, QString("Germany"));
	int austriaID	= addCountry(q, QString("Austria"));
	int franceID	= addCountry(q, QString("France"));
	
	if (!q.prepare(INSERT_RANGE)) return q.lastError();
	int alpsID		= addRange(q, QString("Alps"),		2);
	int pyreneesID	= addRange(q, QString("Pyrenees"),	2);
	
	if (!q.prepare(INSERT_REGION)) return q.lastError();
	int allgauID	= addRegion(q, QString("Allgäu"),				alpsID,	germanyID);
	int vabID		= addRegion(q, QString("Vorarlberg"),			alpsID,	austriaID);
	int vanoiseID	= addRegion(q, QString("Vanoise"),				alpsID,	franceID);
	int pyrenID		= addRegion(q, QString("Pyrénées Ariegéoises"),	pyreneesID,	franceID);
	
	if (!q.prepare(INSERT_PEAK)) return q.lastError();
	int zugspitzeID	= addPeak(q, QString("Zugspitze"),			2456,	false,	allgauID,	QString("maps link 1"),	QString("earth link 1"),	QString("wiki link 1"));
	int montblancID	= addPeak(q, QString("Mont Blanc"),			3435,	false,	vanoiseID,	QString("maps link 2"),	QString("earth link 2"),	QString("wiki link 2"));
	int pfanderID	= addPeak(q, QString("Pfänder"),			1549,	false,	vabID,		QString("maps link 3"),	QString("earth link 3"),	QString("wiki link 3"));
	int brockenID	= addPeak(q, QString("Brocken"),			520,	true,	allgauID,	QString("maps link 4"),	QString("earth link 4"),	QString("wiki link 4"));
	int pyrenMntnID	= addPeak(q, QString("Pyrénées Mountain"),	1894,	false,	pyrenID,	QString("maps link 5"),	QString("earth link 5"),	QString("wiki link 5"));
	
	if (!q.prepare(INSERT_HIKER)) return q.lastError();
	int aliceID		= addHiker(q, QString("Alice"));
	int bobID		= addHiker(q, QString("Bob"));
	int charlieID	= addHiker(q, QString("Charlie"));
	int dorisID		= addHiker(q, QString("Doris"));
	int eileenID	= addHiker(q, QString("Eileen"));
	
	if (!q.prepare(INSERT_TRIP)) return q.lastError();
	int alpsTripID	= addTrip(q, QString("Awesome Alps trip"),			QDate(1999, 12, 29),	QDate(2000, 1, 5),	QString("Wristwatch bugged out"));
	int bandcampID	= addTrip(q, QString("That one time at band camp"),	QDate(2004, 3, 15),		QDate(2004, 4, 2),	QString("We had a pillow fight"));
	int alpsAgainID	= addTrip(q, QString("Alps again"),					QDate(2020, 3, 12),		QDate(2020, 8, 26),	QString("Train back cancelled for some reason"));
	
	if (!q.prepare(INSERT_ASCENT)) return q.lastError();
	int ascent1_1ID	= addAscent(q, QString("Peak of Europe"),		montblancID,	QDate(1999, 12, 29),	1,	QTime(15, 35),	0,	false,	1,	4,	alpsTripID,		QString("Some notes"));
	int ascent1_2ID	= addAscent(q, QString("Lake and stuff"),		pfanderID,		QDate(2000, 1, 2),		1,	QTime(13, 48),	0,	false,	1,	4,	alpsTripID,		QString("Some other notes"));
	int ascent2_1ID	= addAscent(q, QString("A view to a Spain"),	pyrenMntnID,	QDate(2004, 3, 25),		1,	QTime(15, 35),	2,	false,	1,	4,	bandcampID,		QString("Other notes still"));
	int ascent3_1ID	= addAscent(q, QString("Back here, huh?"),		pfanderID,		QDate(2020, 3, 16),		1,	QTime(11, 19),	0,	false,	1,	4,	alpsAgainID,	QString("Yet other notes"));
	int ascent3_2ID	= addAscent(q, QString("Feels wrong"),			brockenID,		QDate(2020, 3, 16),		2,	QTime(16, 5),	3,	true,	1,	4,	alpsAgainID,	QString("These are not the same notes"));
	int ascent3_3ID	= addAscent(q, QString("I can see my house"),	zugspitzeID,	QDate(2020, 7, 19),		1,	QTime(14, 55),	0,	false,	1,	4,	alpsAgainID,	QString("These aren't notes"));
	
	if (!q.prepare(INSERT_PARTICIPATED)) return q.lastError();
	addParticipated(q, ascent1_1ID, aliceID);
	addParticipated(q, ascent1_2ID, aliceID);
	addParticipated(q, ascent1_2ID, charlieID);
	addParticipated(q, ascent1_2ID, eileenID);
	addParticipated(q, ascent2_1ID, aliceID);
	addParticipated(q, ascent2_1ID, bobID);
	addParticipated(q, ascent2_1ID, dorisID);
	addParticipated(q, ascent3_1ID, aliceID);
	addParticipated(q, ascent3_1ID, bobID);
	addParticipated(q, ascent3_1ID, charlieID);
	addParticipated(q, ascent3_1ID, dorisID);
	addParticipated(q, ascent3_1ID, eileenID);
	addParticipated(q, ascent3_2ID, aliceID);
	addParticipated(q, ascent3_2ID, bobID);
	addParticipated(q, ascent3_2ID, dorisID);
	addParticipated(q, ascent3_3ID, aliceID);
	
	if (!q.prepare(INSERT_HIKER)) return q.lastError();
	addPhoto(q, ascent1_1ID, 0, QString("M:/Photos/1999/Alps/2358.jpg"));
	addPhoto(q, ascent1_1ID, 1, QString("M:/Photos/1999/Alps/2363.jpg"));
	addPhoto(q, ascent1_2ID, 0, QString("M:/Photos/1999/Alps/2834.jpg"));
	addPhoto(q, ascent1_2ID, 1, QString("M:/Photos/1999/Alps/2835.jpg"));
	addPhoto(q, ascent1_2ID, 2, QString("M:/Photos/1999/Alps/2798.jpg"));
	addPhoto(q, ascent1_2ID, 3, QString("M:/Photos/1999/Alps/2815.jpg"));
	addPhoto(q, ascent2_1ID, 0, QString("M:/Photos/2004/Band camp/2358.jpg"));
	addPhoto(q, ascent2_1ID, 1, QString("M:/Photos/2004/Band camp/2327.jpg"));
	addPhoto(q, ascent2_1ID, 2, QString("M:/Photos/2004/Band camp/2357.jpg"));
	addPhoto(q, ascent2_1ID, 3, QString("M:/Photos/2004/Band camp/2317.jpg"));
	addPhoto(q, ascent2_1ID, 4, QString("M:/Photos/2004/Band camp/2394.jpg"));
	addPhoto(q, ascent3_1ID, 0, QString("M:/Photos/2020/Alps/1274.jpg"));
	addPhoto(q, ascent3_1ID, 1, QString("M:/Photos/2020/Alps/1274.jpg"));
	addPhoto(q, ascent3_2ID, 0, QString("M:/Photos/2020/Alps/1957.jpg"));
	addPhoto(q, ascent3_3ID, 0, QString("M:/Photos/2020/Alps/2492.jpg"));
	addPhoto(q, ascent3_3ID, 1, QString("M:/Photos/2020/Alps/2452.jpg"));
	addPhoto(q, ascent3_3ID, 2, QString("M:/Photos/2020/Alps/2503.jpg"));
	
	qDebug() << "Database initialized with sample data";
	
	return QSqlError();
}



#endif // TEST_DB_H
