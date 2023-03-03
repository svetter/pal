#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <QtSql>



const auto INSERT_ASCENT		= "INSERT INTO Ascents(title, peakID, date, peakOnDay, time, elevationGain, hikeKind, traverse, difficultySystem, difficultyGrade, tripID, description) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
const auto INSERT_PEAK			= "INSERT INTO Peaks(name, height, volcano, regionID, mapsLink, earthLink, wikiLink) VALUES(?, ?, ?, ?, ?, ?, ?)";
const auto INSERT_TRIP			= "INSERT INTO Trips(name, startDate, endDate, description) VALUES(?, ?, ?, ?)";
const auto INSERT_HIKER			= "INSERT INTO Hikers(name) VALUES(?)";
const auto INSERT_REGION		= "INSERT INTO Regions(name, rangeID, countryID) VALUES(?, ?, ?)";
const auto INSERT_RANGE			= "INSERT INTO Ranges(name, continent) VALUES(?, ?)";
const auto INSERT_COUNTRY		= "INSERT INTO Countries(name) VALUES(?)";
const auto INSERT_PHOTO			= "INSERT INTO Photos(ascentID, sortIndex, filepath, description) VALUES(?, ?, ?, ?)";
const auto INSERT_PARTICIPATED	= "INSERT INTO Participated(ascentID, hikerID) VALUES(?, ?)";



int addAscent(QSqlQuery& q, const QString& title, int peakID, const QDate& date, int peakOnDay, const QTime& time, int elevationGain, int hikeKind, bool traverse, int difficultySystem, int difficultyGrade, int tripID, const QString& description)
{
	q.addBindValue(title);
	q.addBindValue(peakID);
	q.addBindValue(date);
	q.addBindValue(peakOnDay);
	q.addBindValue(time);
	q.addBindValue(elevationGain);
	q.addBindValue(hikeKind);
	q.addBindValue(traverse);
	q.addBindValue(difficultySystem);
	q.addBindValue(difficultyGrade);
	q.addBindValue(tripID);
	q.addBindValue(description);
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

int addTrip(QSqlQuery& q, const QString& name, const QDate& startDate, const QDate& endDate, const QString& description)
{
	q.addBindValue(name);
	q.addBindValue(startDate);
	q.addBindValue(endDate);
	q.addBindValue(description);
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

int addPhoto(QSqlQuery& q, int ascentID, int index, const QString& filepath, const QString& description)
{
	q.addBindValue(ascentID);
	q.addBindValue(index);
	q.addBindValue(filepath);
	q.addBindValue(description);
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



QSqlError insertTestDataIntoSql()
{
	QSqlQuery q;
	
	if (!q.prepare(INSERT_COUNTRY)) return q.lastError();
	int germanyID	= addCountry(q, QString("Germany"));
	int austriaID	= addCountry(q, QString("Austria"));
	int franceID	= addCountry(q, QString("France"));
	
	if (!q.prepare(INSERT_RANGE)) return q.lastError();
	int alpsID		= addRange(q, QString("Alps"),		3);
	int pyreneesID	= addRange(q, QString("Pyrénées"),	3);
	
	if (!q.prepare(INSERT_REGION)) return q.lastError();
	int allgauID	= addRegion(q, QString("Allgäu"),				alpsID,	germanyID);
	int vabID		= addRegion(q, QString("Vorarlberg"),			alpsID,	austriaID);
	int vanoiseID	= addRegion(q, QString("Vanoise"),				alpsID,	franceID);
	int pyrenID		= addRegion(q, QString("Pyrénées Ariegéoises"),	pyreneesID,	franceID);
	
	if (!q.prepare(INSERT_PEAK)) return q.lastError();
	int zugspitzeID	= addPeak(q, QString("Zugspitze"),			2456,	false,	allgauID,	QString("https://goo.gl/maps/jwn8DH2tEZb63dS99"),	QString("https://earth.google.com/web/search/zugspitze"),	QString("https://en.wikipedia.org/wiki/Zugspitze"));
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
	int ascent1_1ID	= addAscent(q, QString("Peak of Europe"),		montblancID,	QDate(1999, 12, 29),	1,	QTime(15, 35),	3470,	0,	false,	1,	4,	alpsTripID,		QString("Some notes"));
	int ascent1_2ID	= addAscent(q, QString("Lake and stuff"),		pfanderID,		QDate(2000, 1, 2),		1,	QTime(13, 48),	1030,	0,	false,	2,	3,	alpsTripID,		QString("Some other notes"));
	int ascent2_1ID	= addAscent(q, QString("A view to a Spain"),	pyrenMntnID,	QDate(2004, 3, 25),		1,	QTime(15, 35),	1950,	2,	false,	1,	4,	bandcampID,		QString("Other notes still"));
	int ascent3_1ID	= addAscent(q, QString("Back here, huh?"),		pfanderID,		QDate(2020, 3, 16),		1,	QTime(11, 19),	950,	0,	false,	4,	6,	alpsAgainID,	QString("Yet other notes"));
	int ascent3_2ID	= addAscent(q, QString("Feels wrong"),			brockenID,		QDate(2020, 3, 16),		2,	QTime(16, 5),	320,	3,	true,	3,	3,	alpsAgainID,	QString("These are not the same notes"));
	int ascent3_3ID	= addAscent(q, QString("I can see my house"),	zugspitzeID,	QDate(2020, 7, 19),		1,	QTime(14, 55),	3333,	0,	false,	2,	1,	alpsAgainID,	QString("These aren't notes"));
	
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
	
	if (!q.prepare(INSERT_PHOTO)) return q.lastError();
	addPhoto(q, ascent1_1ID, 0, QString("M:\\Photos\\1999\\Alps\\2358.jpg"),		"Photo description 1");
	addPhoto(q, ascent1_1ID, 1, QString("M:\\Photos\\1999\\Alps\\2363.jpg"),		"Photo description 2");
	addPhoto(q, ascent1_2ID, 0, QString("M:\\Photos\\1999\\Alps\\2834.jpg"),		"Photo description 3");
	addPhoto(q, ascent1_2ID, 1, QString("M:\\Photos\\1999\\Alps\\2835.jpg"),		"Photo description 4");
	addPhoto(q, ascent1_2ID, 2, QString("M:\\Photos\\1999\\Alps\\2798.jpg"),		"Photo description 5");
	addPhoto(q, ascent1_2ID, 3, QString("M:\\Photos\\1999\\Alps\\2815.jpg"),		"Photo description 6");
	addPhoto(q, ascent2_1ID, 0, QString("M:\\Photos\\2004\\Band camp\\2358.jpg"),	"Photo description 7");
	addPhoto(q, ascent2_1ID, 1, QString("M:\\Photos\\2004\\Band camp\\2327.jpg"),	"Photo description 8");
	addPhoto(q, ascent2_1ID, 2, QString("M:\\Photos\\2004\\Band camp\\2357.jpg"),	"Photo description 9");
	addPhoto(q, ascent2_1ID, 3, QString("M:\\Photos\\2004\\Band camp\\2317.jpg"),	"Photo description 10");
	addPhoto(q, ascent2_1ID, 4, QString("M:\\Photos\\2004\\Band camp\\2394.jpg"),	"Photo description 11");
	addPhoto(q, ascent3_1ID, 0, QString("M:\\Photos\\2020\\Alps\\1274.jpg"),		"Photo description 12");
	addPhoto(q, ascent3_1ID, 1, QString("M:\\Photos\\2020\\Alps\\1275.jpg"),		"Photo description 13");
	addPhoto(q, ascent3_2ID, 0, QString("M:\\Photos\\2020\\Alps\\1957.jpg"),		"Photo description 14");
	addPhoto(q, ascent3_3ID, 0, QString("M:\\Photos\\2020\\Alps\\2492.jpg"),		"Photo description 15");
	addPhoto(q, ascent3_3ID, 1, QString("M:\\Photos\\2020\\Alps\\2452.jpg"),		"Photo description 16");
	addPhoto(q, ascent3_3ID, 2, QString("M:\\Photos\\2020\\Alps\\2503.jpg"),		"Photo description 17");
	
	qDebug() << "Test data added to database";
	
	return QSqlError();
}



#endif // TEST_DATA_H
